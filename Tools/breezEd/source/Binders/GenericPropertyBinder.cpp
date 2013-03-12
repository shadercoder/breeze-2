#include "stdafx.h"
#include "Binders/GenericPropertyBinder.h"

#include <QtWidgets/QTreeView>

#include <beCore/bePropertyVisitor.h>
#include <beCore/beValueType.h>
#include <beCore/beTextSerializer.h>
#include <sstream>

#include "Commands/ChangeProperty.h"
#include "Documents/SceneDocument.h"

#include <beCore/beComponentTypes.h>
#include "Commands/SetComponent.h"

#include "Editor.h" // Dependencies?!!
#include "Tiles/ComponentBrowserWidget.h"

#include "Widgets/ComponentPicker.h"
#include "Widgets/ComponentPickerFactory.h"
#include "Plugins/FactoryManager.h"

#include "Widgets/ColorPicker.h"

#include "Utility/ItemDelegateEx.h"

#include <lean/smart/scoped_ptr.h>
#include <QtCore/QVariant>

#include <lean/io/filesystem.h>

#include "Utility/SlotObject.h"
#include "Utility/InlineEditor.h"

#include "Utility/Undo.h"
#include "Utility/Strings.h"
#include "Utility/Checked.h"

Q_DECLARE_METATYPE(GenericPropertyBinder*);

namespace
{

/// Serializes any visited property & writes the results to the stored strings.
struct PropertyValueReader : public beCore::DataVisitor
{
	QString value;
	QVector<QString> components;

	/// Visits the given values.
	void Visit(const bec::ValueTypeDesc &typeDesc, const void *values, size_t count) LEAN_OVERRIDE
	{
		const beCore::TextSerializer *pSerializer = typeDesc.Text;

		if (pSerializer)
		{
			std::stringstream str;
			
			value = (pSerializer->Write(str, typeDesc.Info.type, values, count))
				?  toQt( str.str() )
				: GenericPropertyBinder::tr("<error>");

			if (count > 1)
			{
				components.reserve( static_cast<int>(count) );

				for (size_t i = 0; i < count; ++i)
				{
					std::stringstream str;

					components.push_back( 
						(pSerializer->Write(str, typeDesc.Info.type, lean::get_element(typeDesc.Info.size, values, i), 1))
							?  toQt( str.str() )
							: GenericPropertyBinder::tr("<error>")
						);
				}
			}
		}
		else
			value = GenericPropertyBinder::tr("<error>");
	}
};

/// Parses the stored string & writes the results to any visited property.
struct PropertyValueWriter : public beCore::DataVisitor
{
	QString valueString;
	uint4 componentIdx;

	/// Constructor.
	PropertyValueWriter(const QString &valueString, uint4 componentIdx = static_cast<uint4>(-1))
		: valueString(valueString),
		componentIdx(componentIdx) { }

	/// Visits the given values.
	bool Visit(const bec::ValueTypeDesc &typeDesc, void *values, size_t count) LEAN_OVERRIDE
	{
		bool success = false;

		const beCore::TextSerializer *pSerializer = typeDesc.Text;

		if (pSerializer)
		{
			std::stringstream valueStream( toUtf8(valueString) );

			if (componentIdx < count)
				success = pSerializer->Read(
						valueStream,
						typeDesc.Info.type, lean::get_element(typeDesc.Info.size, values, componentIdx), 1
					);
			else
				success = pSerializer->Read(
						valueStream,
						typeDesc.Info.type, values, count
					);

			if (!success)
				QMessageBox::critical(
					nullptr,
					GenericPropertyBinder::tr("Invalid value"),
					GenericPropertyBinder::tr("An error occurred while trying to interpret value '%1'.").arg(valueString)
				);
		}
		else
			QMessageBox::critical(
					nullptr,
					GenericPropertyBinder::tr("Serializer unavailable"),
					GenericPropertyBinder::tr("No serializer available for property type '%1'.").arg( toQt(typeDesc.Name) )
				);

		return success;
	}
};

/// Gets a printable representation of the given component.
QString getComponentValue(const beCore::ReflectedComponent &reflectedComponent, uint4 componentIdx)
{
	QString name;

	const beCore::ComponentReflector *pReflector = beCore::GetComponentTypes().GetReflector(
			reflectedComponent.GetComponentType(componentIdx)
		);

	if (pReflector)
	{
		beCore::ComponentInfo info = pReflector->GetInfo(*reflectedComponent.GetComponent(componentIdx));
		
		if (!info.Name.empty())
			name = toQt(info.Name);
		else
			name = GenericPropertyBinder::tr("<Not set / Internal>");
	}
	else
		name = GenericPropertyBinder::tr("<Unknown type>");

	return name;
}

/// Adds the properties of the given provider.
void addProperties(beCore::PropertyProvider &propertyProvider, QStandardItem &parentItem)
{
	const uint4 propertyCount = propertyProvider.GetPropertyCount();

	for (uint4 i = 0; i < propertyCount; ++i)
	{
		QString propertyName = toQt( propertyProvider.GetPropertyName(i) );
		
		PropertyValueReader propertyReader;
		propertyProvider.ReadProperty(i, beCore::PropertyDataVisitor(propertyReader));

		beCore::PropertyDesc desc = propertyProvider.GetPropertyDesc(i);
		Qt::ItemFlags editableFlag = (desc.Widget != beCore::Widget::None) ? Qt::ItemIsEditable : 0;

		// Property name & value
		QStandardItem *pNameItem = new QStandardItem(propertyName);
		pNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

		QStandardItem *pValueItem = new QStandardItem(propertyReader.value);
		pValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | editableFlag);
		pValueItem->setData( QVariant::fromValue(i) );
		
		parentItem.appendRow( QList<QStandardItem*>() << pNameItem << pValueItem );

		

		// Only add single component items if more than one
		if (propertyReader.components.size() > 1)
		{
			const uint4 componentCount = propertyReader.components.size();

			// Alternative components names
			static const char *const vectorComponentNames[] = { "x", "y", "z", "w" };
			static const char *const colorComponentNames[] = { "r", "g", "b", "a" };
			const char *const *componentNames = nullptr;

			if (componentCount <= 4)
				componentNames = (desc.Widget == beCore::Widget::Color) ? colorComponentNames : vectorComponentNames;

			for (uint4 c = 0; c < componentCount; ++c)
			{
				// Component name & value
				QStandardItem *pComponentNameItem = new QStandardItem( (componentNames) ? componentNames[c] : QString("[%1]").arg(c) );
				pComponentNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				
				QStandardItem *pComponentValueItem = new QStandardItem(propertyReader.components[c]);
				pComponentValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | editableFlag);
				
				pNameItem->appendRow( QList<QStandardItem*>() << pComponentNameItem << pComponentValueItem );
			}
		}
	}
}

/// Adds the child components of the given provider.
void addComponents(beCore::ReflectedComponent &reflectedComponent, QStandardItem &parentItem, QTreeView &tree, SceneDocument *pDocument, GenericPropertyBinder *pBinder)
{
	const uint4 componentCount = reflectedComponent.GetComponentCount();
	
	for (uint4 i = 0; i < componentCount; ++i)
	{
		QString name = toQt( reflectedComponent.GetComponentName(i) );
		bool bEditable = reflectedComponent.IsComponentReplaceable(i);

		QStandardItem *pNameItem = new QStandardItem(name);

		// Make fixed components non-interactive
		if (!bEditable)
		{
			pNameItem->setFlags(Qt::ItemIsEnabled);
			parentItem.appendRow(pNameItem);
			GenericPropertyBinder::fillRow(*pNameItem);
		}
		// Make replaceable components interactive & editable
		else
		{
			pNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

			QStandardItem *pValueItem = new QStandardItem( getComponentValue(reflectedComponent, i) );
			pValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			parentItem.appendRow( QList<QStandardItem*>() << pNameItem << pValueItem );
		}

		lean::com_ptr<beCore::ReflectedComponent> pChildComponent = reflectedComponent.GetReflectedComponent(i);
		// NOTE: Make sure we use the same reflection object
		lean::com_ptr<beCore::PropertyProvider> pChildProvider = pChildComponent;
		if (!pChildProvider) pChildProvider = reflectedComponent.GetPropertyProvider(i);

		// WARNING: May be null
		if (pChildProvider)
		{
			// Add child component sub-tree
			GenericPropertyBinder *pChildBinder = new GenericPropertyBinder(pChildProvider, pChildComponent, pDocument, &tree, pNameItem, pBinder);
			checkedConnect(pBinder, SIGNAL(propagateUpdateProperties()), pChildBinder, SLOT(updateProperties()));
			checkedConnect(pChildBinder, SIGNAL(propertiesChanged()), pBinder, SIGNAL(propertiesChanged()));

			pNameItem->setData( QVariant::fromValue(pChildBinder) );

			if (reflectedComponent.IsComponentEssential(i) && componentCount <= 3)
				tree.expand( pNameItem->index() );
		}
	}
}

} // namespace

// Constructor.
GenericPropertyBinder::GenericPropertyBinder(beCore::PropertyProvider *pPropertyProvider,
											 beCore::ReflectedComponent *pComponent,
											 SceneDocument *pDocument,
											 QTreeView *pTree, QStandardItem *pParentItem,
											 QObject *pParent)
	: QObject(pParent),
	m_pPropertyProvider( LEAN_ASSERT_NOT_NULL(pPropertyProvider) ),
	m_pComponent( pComponent ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pTree( LEAN_ASSERT_NOT_NULL(pTree) ),
	m_pParentItem( pParentItem ),
	m_propertyStartIdx(),
	m_propertyEndIdx(),
	m_componentStartIdx(),
	m_componentEndIdx()
{
	LEAN_ASSERT(!m_pComponent || m_pComponent == m_pPropertyProvider);

	// Create new empty model, if none to be extended
	if (!m_pParentItem)
	{
		GenericPropertyBinder::setupTree(*m_pTree);

		// WARNING: Don't attach to binder, binder might be attached to model
		QStandardItemModel *pModel = new QStandardItemModel(m_pTree);
		setupModel(*pModel);
		m_pTree->setModel(pModel);

		m_pParentItem = pModel->invisibleRootItem();
	}

	// Get extended item delegate
	m_pDelegate = qobject_cast<ItemDelegateEx*>( m_pTree->itemDelegate() );

	// Catch trees not properly set up
	if (!m_pDelegate)
	{
		qWarning("GenericPropertyBinder: Extendend item delegate missing, overriding now");
		setupTree(*m_pTree);
		m_pDelegate = qobject_cast<ItemDelegateEx*>( m_pTree->itemDelegate() );
	}

	// Properties
	m_propertyStartIdx = m_pParentItem->rowCount();
	addProperties(*m_pPropertyProvider, *m_pParentItem);
	m_propertyEndIdx = m_pParentItem->rowCount();

	// Components
	m_componentStartIdx = m_pParentItem->rowCount();
	
	if (m_pComponent)
		addComponents(*m_pComponent, *m_pParentItem, *m_pTree, m_pDocument, this);

	m_componentEndIdx = m_pParentItem->rowCount();

	// WARNING: Direct connection is dangerous, crashes everything on inferred editor focus loss
	checkedConnect(
			m_pDelegate, SIGNAL(dataUpdated(QAbstractItemModel*, const QModelIndex&)),
			this, SLOT(dataUpdated(QAbstractItemModel*, const QModelIndex&)),
			Qt::QueuedConnection
		);

	// WARNING: Signals require IMMEDIATE feedback (due to 'handled' parameters)
	checkedConnect(
			m_pDelegate, SIGNAL(startEditing(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, QWidget*&, bool&)),
			this, SLOT(startEditing(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, QWidget*&, bool&)),
			Qt::DirectConnection
		);
	checkedConnect(
			m_pDelegate, SIGNAL(updateEditor(QWidget*, const QModelIndex&, bool&)),
			this, SLOT(updateEditor(QWidget*, const QModelIndex&, bool&)),
			Qt::DirectConnection
		);
	checkedConnect(
			m_pDelegate, SIGNAL(updateData(QWidget*, QAbstractItemModel*, const QModelIndex&, bool&)),
			this, SLOT(updateData(QWidget*, QAbstractItemModel*, const QModelIndex&, bool&)),
			Qt::DirectConnection
		);
	checkedConnect(
			m_pDelegate, SIGNAL(editorGeometryChanged(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, bool&)),
			this, SLOT(updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, bool&)),
			Qt::DirectConnection
		);

	// ORDER: Always last, afterwards destructor HAS TO BE CALLED
	m_pPropertyProvider->AddObserver(this);
}

// Destructor.
GenericPropertyBinder::~GenericPropertyBinder()
{
	disconnect(this, SLOT(updateProperties()));
	disconnect(this, SIGNAL(propertiesChanged()));

	m_pPropertyProvider->RemoveObserver(this);
}

// Check for property changes.
void GenericPropertyBinder::updateProperties()
{
	if (m_bPropertiesChanged)
	{
		// Update properties.
		for (int row = m_propertyStartIdx; row < m_propertyEndIdx; ++row)
		{
			uint4 propertyID = static_cast<uint4>(row - m_propertyStartIdx);

			PropertyValueReader propertyReader;
			m_pPropertyProvider->ReadProperty(propertyID, beCore::PropertyDataVisitor(propertyReader));

			// Update property item
			QStandardItem *pPropertyValue = m_pParentItem->child(row, 1);
			pPropertyValue->setText(propertyReader.value);

			// Update property component items
			if (propertyReader.components.size() > 1)
			{
				QStandardItem *pPropertyName = m_pParentItem->child(row, 0);

				const int componentCount = propertyReader.components.size();

				for (int c = 0; c < componentCount; ++c)
					pPropertyName->child(c, 1)->setText(propertyReader.components[c]);
			}
		}

		// IMPORTANT TODO: Update component count?!?!

		// Update components
		for (int row = m_componentStartIdx; row < m_componentEndIdx; ++row)
		{
			uint4 componentIdx = static_cast<uint4>(row - m_componentStartIdx);
			bool bEditable = m_pComponent->IsComponentReplaceable(componentIdx);

			// Update component item
			if (bEditable)
			{
				QStandardItem *pComponentValue = m_pParentItem->child(row, 1);
				pComponentValue->setText( getComponentValue(*m_pComponent, componentIdx) );
			}

			// Update component properties
			QStandardItem *pComponentItem = m_pParentItem->child(row, 0);

			GenericPropertyBinder *pChildBinder = pComponentItem->data().value<GenericPropertyBinder*>();
			const beCore::PropertyProvider *pPreviousChildProvider = (pChildBinder) ? pChildBinder->propertyProvider() : nullptr;
			
			lean::com_ptr<beCore::ReflectedComponent> pChildComponent = m_pComponent->GetReflectedComponent(componentIdx);
			lean::com_ptr<beCore::PropertyProvider> pChildProvider = pChildComponent;
			if (!pChildProvider) pChildProvider = m_pComponent->GetPropertyProvider(componentIdx);

			// Component change results in sub-tree rebuild
			// -> Only update if actually changed
			if (pChildProvider != pPreviousChildProvider)
			{
				// WARNING: Delete IMMEDIATELY, prevent any further update events (bound object might already have been deleted)
				delete pChildBinder;
				pChildBinder = nullptr;
				
				// Clear component sub-tree
				pComponentItem->setData( QVariant::fromValue(pChildBinder) );
				pComponentItem->setRowCount(0);

				// Replace with sub-tree of new child component
				pChildBinder = new GenericPropertyBinder(pChildProvider, pChildComponent, m_pDocument, m_pTree, pComponentItem, this);
				checkedConnect(pChildBinder, SIGNAL(propertiesChanged()), this, SIGNAL(propertiesChanged()));
				checkedConnect(this, SIGNAL(propagateUpdateProperties()), pChildBinder, SLOT(updateProperties()));

				pComponentItem->setData( QVariant::fromValue(pChildBinder) );
			}
		}

		m_bPropertiesChanged = false;
	}

	Q_EMIT propagateUpdateProperties();
}

// Called when properties in the given provider might have changed.
void GenericPropertyBinder::PropertyChanged(const beCore::PropertyProvider &provider)
{
	m_bPropertiesChanged = true;
	Q_EMIT propertiesChanged();
}

// Called when child components in the given provider might have changed.
void GenericPropertyBinder::ChildChanged(const beCore::ReflectedComponent &provider)
{
	m_bPropertiesChanged = true;
	Q_EMIT propertiesChanged();
}

// Called when the structure of the given component has changed.
void GenericPropertyBinder::StructureChanged(const beCore::Component &provider)
{
	// TODO
}

// Called when the given component has been replaced.
void GenericPropertyBinder::ComponentReplaced(const beCore::Component &previous)
{
	// TODO
}

// Data has changed.
void GenericPropertyBinder::dataUpdated(QAbstractItemModel *model, const QModelIndex &index)
{
	QModelIndex rootIndex = m_pParentItem->index();
	QModelIndex parentIndex = index.parent();

	// Property
	if (parentIndex == rootIndex)
	{
		int row = index.row();

		if (m_propertyStartIdx <= row && row < m_propertyEndIdx)
		{
			uint4 propertyID = static_cast<uint4>( row - m_propertyStartIdx );

			beCore::PropertyDesc desc = m_pPropertyProvider->GetPropertyDesc(propertyID);

			// Only accept vector string changes for raw values
			if (desc.Widget == beCore::Widget::Raw || desc.Widget == beCore::Widget::Slider)
			// TODO: NEED some form of text editing support!
			{
				QStandardItem *pValueItem = m_pParentItem->child(row, 1);

				// Capture current property value
				lean::scoped_ptr<ChangePropertyCommand> command( new ChangePropertyCommand(m_pPropertyProvider, propertyID) );

				// Read data back & write to property provider
				PropertyValueWriter propertyWriter(pValueItem->text());
				m_pPropertyProvider->WriteProperty(propertyID, beCore::PropertyDataVisitor(propertyWriter));

				// Capture new property value
				command->capture();
				m_pDocument->undoStack()->push( command.detach()->pushOnly() );
			}
		}
	}
	// Property component
	else if (parentIndex.parent() == rootIndex)
	{
		int propertyRow = parentIndex.row();

		if (m_propertyStartIdx <= propertyRow && propertyRow < m_propertyEndIdx)
		{
			uint4 propertyID = static_cast<uint4>( propertyRow - m_propertyStartIdx );
			uint4 component = static_cast<uint4>( index.row() );

			QStandardItem *pValueItem = m_pParentItem->child(propertyRow, 0)->child(index.row(), 1);

			// Capture current property value
			lean::scoped_ptr<ChangePropertyCommand> command( new ChangePropertyCommand(m_pPropertyProvider, propertyID) );

			// Read single component back & write to property provider
			PropertyValueWriter propertyWriter(pValueItem->text(), component);
			m_pPropertyProvider->WriteProperty(propertyID, beCore::PropertyDataVisitor(propertyWriter), bec::PropertyVisitFlags::PartialWrite);
			
			// Capture new property value
			command->capture();
			m_pDocument->undoStack()->push( command.detach() );
		}
	}
}

namespace
{

class InlineColorEditor : public InlineEditor
{
private:
	SceneDocument *m_document;
	beCore::PropertyProvider *m_propertyProvider;
	uint4 m_propertyID;

	ColorPicker *m_picker;

	ChangePropertyCommand *m_pCommand;

protected:
	/// Updates the color property being edited.
	void colorChanged()
	{
		if (!m_pCommand)
		{
			// Capture current property value & start editing
			m_pCommand = new ChangePropertyCommand(m_propertyProvider, m_propertyID);
			m_document->undoStack()->push( m_pCommand->pushOnly() );
		}
		else
			redoIfMostRecent(*m_document->undoStack(), *m_pCommand);

		// Update property
		m_propertyProvider->SetProperty(m_propertyID, m_picker->color().cdata(), 4);
		m_propertyProvider->ForcedChangeHint();

		// Capture new property value
		m_pCommand->capture();
	}

public:
	InlineColorEditor(SceneDocument *document, beCore::PropertyProvider *provider, uint4 propertyID, QWidget *pParent)
		: InlineEditor(true, pParent),
		m_document( LEAN_ASSERT_NOT_NULL(document) ),
		m_propertyProvider( LEAN_ASSERT_NOT_NULL(provider) ),
		m_propertyID( propertyID ),
		m_pCommand()
	{
		m_picker = new ColorPicker(1.0e6f, this);
		this->setWidget(m_picker);

		/// Initialize color
		beMath::fvec4 color;
		m_propertyProvider->GetProperty(m_propertyID, color.data(), 4);
		m_picker->setColor(color);

		connect(m_picker, &ColorPicker::colorChanged, this, &InlineColorEditor::colorChanged);
	}
	/// Discards unaccepted editing results.
	~InlineColorEditor()
	{
		cancel();
	}

	/// Accepts the editing results.
	void accept() LEAN_OVERRIDE
	{
		if (m_pCommand)
		{
			redoIfMostRecent(*m_document->undoStack(), *m_pCommand);
			m_pCommand = nullptr;
		}
	}
	/// Discards the editing results.
	void cancel()
	{
		if (m_pCommand)
			undoIfMostRecent(*m_document->undoStack(), *m_pCommand);
	}
};

class InlineComponentEditor : public InlineEditor
{
private:
	SceneDocument *m_document;
	beCore::ReflectedComponent *m_componentProvider;
	uint4 m_componentID;

	ComponentSelectorWidget *m_selector;

	SetComponentCommand *m_pCommand;

protected:
	/// Updates the color property being edited.
	void componentChanged(const lean::any *pComponent)
	{
		cancel();

		if (pComponent)
		{
			// Capture current property value & start editing
			m_pCommand = new SetComponentCommand(m_componentProvider, m_componentID, *pComponent, m_document->graphicsResources()->Monitor());
			m_document->undoStack()->push(m_pCommand);
		}
	}

public:
	InlineComponentEditor(const beCore::ComponentTypeDesc *typeDesc, QAbstractItemDelegate *delegate, SceneDocument *document,
						  beCore::ReflectedComponent *componentProvider, uint4 componentID, QWidget *pParent)
		: InlineEditor(false, pParent),
		m_document( LEAN_ASSERT_NOT_NULL(document) ),
		m_componentProvider( LEAN_ASSERT_NOT_NULL(componentProvider) ),
		m_componentID( componentID ),
		m_pCommand()
	{
		lean::cloneable_obj<lean::any, lean::ptr_sem> pComponent = m_componentProvider->GetComponent(m_componentID);

		// Open component browser
		bool bLayoutChanged = true;
		ComponentBrowserWidget *browser = retrieveBrowser(typeDesc, m_document->editor()->mainWindow(), pParent, &bLayoutChanged);

		// Move cursor to current item
		if (!bLayoutChanged)
			browser->moveToCurrent();

		// ORDER: Initialize browser
		if (pComponent)
			browser->selectComponent(pComponent);

		// ORDER: Link to browser
		m_selector = new ComponentSelectorWidget(browser, this);
		this->setWidget(m_selector);

		if (pComponent)
			m_selector->setComponent(pComponent, toQt(typeDesc->Reflector->GetInfo(*pComponent).Name));

		this->connectToDelegate(delegate);
		connect(m_selector, &ComponentSelectorWidget::selectionFinished, this, &InlineEditor::commitAndClose);
		connect(m_selector, &ComponentSelectorWidget::componentChanged, this, &InlineComponentEditor::componentChanged);
	}
	/// Discards unaccepted editing results.
	~InlineComponentEditor()
	{
		cancel();
	}

	/// Accepts the editing results.
	void accept() LEAN_OVERRIDE
	{
		if (m_pCommand)
		{
			redoIfMostRecent(*m_document->undoStack(), *m_pCommand);
			m_pCommand = nullptr;
		}
	}
	/// Discards the editing results.
	void cancel()
	{
		if (m_pCommand)
			undoIfMostRecent(*m_document->undoStack(), *m_pCommand);
	}
};

} // namespace

// Editing has started.
void GenericPropertyBinder::startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled)
{
	// Property or component
	if (index.parent() == m_pParentItem->index())
	{
		int row = index.row();

		// Property
		if (m_propertyStartIdx <= row && row < m_propertyEndIdx)
		{
			uint4 propertyID = static_cast<uint4>( row - m_propertyStartIdx );

			beCore::PropertyDesc desc = m_pPropertyProvider->GetPropertyDesc(propertyID);

			// Open color picker
			if (desc.Widget == beCore::Widget::Color)
				pEditor = new InlineColorEditor(m_pDocument, m_pPropertyProvider, propertyID, parent);
		}
		// Component
		else if (m_componentStartIdx <= row && row < m_componentEndIdx)
		{
			uint4 componentIdx = static_cast<uint4>( row - m_componentStartIdx );

			const beCore::ComponentType *componentType = m_pComponent->GetComponentType(componentIdx);
			const beCore::ComponentTypeDesc *pComponentTypeDesc = beCore::GetComponentTypes().GetDesc(componentType);
			
			if (pComponentTypeDesc && pComponentTypeDesc->Reflector)
				pEditor = new InlineComponentEditor(pComponentTypeDesc, m_pDelegate, m_pDocument, m_pComponent, componentIdx, parent);
			else
				QMessageBox::critical(nullptr,
						tr("Unknown component type"),
						tr("Component type '%1' is unknown.").arg( componentType->Name )
					);

			// NOTE: Also handled if no editor has been created
			bHandled = true;
		}
	}
}

// Data currently being edited has changed.
void GenericPropertyBinder::updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled)
{
	if (qobject_cast<InlineEditor*>(editor))
	{
		editor->setFocus();
		bHandled = true;
	}

	// NOTE: Currently, custom editors are never updated after they've been opened
}

// Editor data is requested.
void GenericPropertyBinder::updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled)
{
	if (InlineEditor *inlineEditor = qobject_cast<InlineEditor*>(editor))
	{
		// Never let custom editors leak garbage via default editor handling
		bHandled = true;

		// Finish live editing
		inlineEditor->accept();
	}
}

// Editor to be relocated.
void GenericPropertyBinder::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled)
{
	if (InlineEditor *inlineEditor = qobject_cast<InlineEditor*>(editor))
	{
		if (inlineEditor->isOverlay())
		{
			// Use almost entire with (leave some small space for click escape)
			int itemWidth = option.rect.width();
			int width = (3 * editor->parentWidget()->width() + itemWidth) / 4;

			int height = editor->heightForWidth(width);
			if (height < 0) height = editor->height();

			QPoint editorPos(option.rect.left() - (width - itemWidth), option.rect.bottom());

			// Put editor below property
			editor->move(editorPos); // DOESNT WORK: editor->mapFromGlobal( editor->parentWidget()->mapToGlobal(editorPos) )
			editor->resize(width, height);
		}
		else
			editor->setGeometry(option.rect);

		bHandled = true;
	}
}

// Sets the given tree view up for property editing.
void GenericPropertyBinder::setupTree(QTreeView &view)
{
	ItemDelegateEx *pItemDelegate = qobject_cast<ItemDelegateEx*>( view.itemDelegate() );
	
	// This binder needs the enhanced item delegate
	if (!pItemDelegate)
	{
		QAbstractItemDelegate *pOldDelegate = view.itemDelegate();

		view.setItemDelegate( new ItemDelegateEx(&view) );

		if (pOldDelegate && pOldDelegate->parent() == &view)
			delete pOldDelegate;
	}
}

// Sets the given item model up for property display.
void GenericPropertyBinder::setupModel(QStandardItemModel &model)
{
	// Set up two columns for name & value
	model.setHorizontalHeaderLabels( QStringList()
			<< GenericPropertyBinder::tr("Name")
			<< GenericPropertyBinder::tr("Value") );
}

// Correctly fills empty cells in the given row.
void GenericPropertyBinder::fillRow(QStandardItem &rowItem)
{
	QStandardItem *pRoot = rowItem.model()->invisibleRootItem();
	QStandardItem *pParent = rowItem.parent();

	if (!pParent)
		pParent = pRoot;

	const int columnCount = pRoot->columnCount();
	const int row = rowItem.row();

	// Fill empty columns with dummy cells matching the given item's flags
	for (int i = 0; i < columnCount; ++i)
	{
		QStandardItem *pCellItem = pParent->child(row, i);

		if (!pCellItem)
		{
			pCellItem = new QStandardItem();
			pCellItem->setFlags(Qt::NoItemFlags);
			pCellItem->setEnabled( rowItem.isEnabled() );
			pCellItem->setSelectable( rowItem.isSelectable() );
			pParent->setChild(row, i, pCellItem);
		}
	}
}
