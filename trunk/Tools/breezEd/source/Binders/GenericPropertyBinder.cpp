#include "stdafx.h"
#include "Binders/GenericPropertyBinder.h"

#include <QtGui/QTreeView>

#include <beCore/bePropertyVisitor.h>
#include <beCore/beTextSerialization.h>
#include <sstream>

#include "Commands/ChangeProperty.h"
#include "Documents/SceneDocument.h"

#include <beCore/beComponentTypes.h>
#include "Widgets/ComponentSelectorWidget.h"
#include "Commands/SetComponent.h"

#include "Utility/ItemDelegateEx.h"

#include <lean/smart/scoped_ptr.h>
#include <QtCore/QVariant>

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
	void Visit(uint4 typeID, const lean::type_info &typeInfo, const void *values, size_t count)
	{
		const beCore::TextSerializer *pSerializer = beCore::GetTextSerialization().GetSerializer(typeID);

		if (pSerializer)
		{
			std::stringstream str;
			
			value = (pSerializer->Write(str, typeInfo.type, values, count))
				?  toQt( str.str() )
				: GenericPropertyBinder::tr("<error>");

			if (count > 1)
			{
				components.reserve( static_cast<int>(count) );

				for (size_t i = 0; i < count; ++i)
				{
					std::stringstream str;

					components.push_back( 
						(pSerializer->Write(str, typeInfo.type, lean::get_element(typeInfo.size, values, i), 1))
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
	bool Visit(uint4 typeID, const lean::type_info &typeInfo, void *values, size_t count)
	{
		bool success = false;

		const beCore::TextSerializer *pSerializer = beCore::GetTextSerialization().GetSerializer(typeID);

		if (pSerializer)
		{
			std::stringstream valueStream( toUtf8(valueString) );

			if (componentIdx < count)
				success = pSerializer->Read(
						valueStream,
						typeInfo.type, lean::get_element(typeInfo.size, values, componentIdx), 1
					);
			else
				success = pSerializer->Read(
						valueStream,
						typeInfo.type, values, count
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
					GenericPropertyBinder::tr("No serializer available for property type '%1'.").arg( toQt(beCore::GetTypeIndex().GetName(typeID)) )
				);

		return success;
	}
};

/// Adds the properties of the given provider.
void AddProperties(beCore::PropertyProvider &propertyProvider, QStandardItem &parentItem)
{
	const uint4 propertyCount = propertyProvider.GetPropertyCount();

	for (uint4 i = 0; i < propertyCount; ++i)
	{
		QString propertyName = toQt( propertyProvider.GetPropertyName(i) );
		
		PropertyValueReader propertyReader;
		propertyProvider.ReadProperty(i, beCore::PropertyDataVisitor(propertyReader));

		// Property name & value
		QStandardItem *pNameItem = new QStandardItem(propertyName);
		pNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		
		QStandardItem *pValueItem = new QStandardItem(propertyReader.value);
		pValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		pValueItem->setData( QVariant::fromValue(i) );
		
		parentItem.appendRow( QList<QStandardItem*>() << pNameItem << pValueItem );

		beCore::PropertyDesc desc = propertyProvider.GetPropertyDesc(i);

		// Only add single component items if more than one
		if (propertyReader.components.size() > 1)
		{
			const uint4 componentCount = propertyReader.components.size();

			// Alternative components names
			static const char *const vectorComponentNames[] = { "x", "y", "z", "w" };
			static const char *const colorComponentNames[] = { "r", "g", "b", "a" };
			const char *const *componentNames = nullptr;

			// TODO: Widget?
			if (componentCount <= 4)
				componentNames = vectorComponentNames;

			for (uint4 c = 0; c < componentCount; ++c)
			{
				// Component name & value
				QStandardItem *pComponentNameItem = new QStandardItem( (componentNames) ? componentNames[c] : QString("[%1]").arg(c) );
				pComponentNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				
				QStandardItem *pComponentValueItem = new QStandardItem(propertyReader.components[c]);
				pComponentValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
				
				pNameItem->appendRow( QList<QStandardItem*>() << pComponentNameItem << pComponentValueItem );
			}
		}
	}
}

/// Adds the child components of the given provider.
void AddComponents(beCore::ReflectedComponent &reflectedComponent, QStandardItem &parentItem, QTreeView &tree, SceneDocument *pDocument, GenericPropertyBinder *pBinder)
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

			QStandardItem *pValueItem = new QStandardItem( "??? TODO" );
			pValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			parentItem.appendRow( QList<QStandardItem*>() << pNameItem << pValueItem );
		}

		beCore::PropertyProvider *pChildProvider = reflectedComponent.GetPropertyProvider(i);
		beCore::ReflectedComponent *pChildComponent = reflectedComponent.GetReflectedComponent(i);

		// WARNING: May be null
		if (pChildProvider)
		{
			// Add child component sub-tree
			GenericPropertyBinder *pChildBinder = new GenericPropertyBinder(pChildProvider, pChildComponent, pDocument, &tree, pNameItem, pBinder);
			checkedConnect(pBinder, SIGNAL(propagateUpdateProperties()), pChildBinder, SLOT(updateProperties()));

			pNameItem->setData( QVariant::fromValue(pChildBinder) );

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
	ItemDelegateEx *pDelegate = qobject_cast<ItemDelegateEx*>( m_pTree->itemDelegate() );

	// Catch trees not properly set up
	if (!pDelegate)
	{
		qWarning("GenericPropertyBinder: Extendend item delegate missing, overriding now");
		setupTree(*m_pTree);
		pDelegate = qobject_cast<ItemDelegateEx*>( m_pTree->itemDelegate() );
	}

	// Properties
	m_propertyStartIdx = m_pParentItem->rowCount();
	AddProperties(*m_pPropertyProvider, *m_pParentItem);
	m_propertyEndIdx = m_pParentItem->rowCount();

	// Components
	m_componentStartIdx = m_pParentItem->rowCount();
	
	if (m_pComponent)
		AddComponents(*m_pComponent, *m_pParentItem, *m_pTree, m_pDocument, this);

	m_componentEndIdx = m_pParentItem->rowCount();

	// WARNING: Direct connection is dangerous, crashes everything on inferred editor focus loss
	checkedConnect(
			pDelegate, SIGNAL(dataUpdated(QAbstractItemModel*, const QModelIndex&)),
			this, SLOT(dataUpdated(QAbstractItemModel*, const QModelIndex&)),
			Qt::QueuedConnection
		);

	// WARNING: Signals require IMMEDIATE feedback (due to 'handled' parameters)
	checkedConnect(
			pDelegate, SIGNAL(startEditing(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, QWidget*&, bool&)),
			this, SLOT(startEditing(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, QWidget*&, bool&)),
			Qt::DirectConnection
		);
	checkedConnect(
			pDelegate, SIGNAL(updateEditor(QWidget*, const QModelIndex&, bool&)),
			this, SLOT(updateEditor(QWidget*, const QModelIndex&, bool&)),
			Qt::DirectConnection
		);
	checkedConnect(
			pDelegate, SIGNAL(updateData(QWidget*, QAbstractItemModel*, const QModelIndex&, bool&)),
			this, SLOT(updateData(QWidget*, QAbstractItemModel*, const QModelIndex&, bool&)),
			Qt::DirectConnection
		);
	checkedConnect(
			pDelegate, SIGNAL(updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, bool&)),
			this, SLOT(updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&, bool&)),
			Qt::DirectConnection
		);

	// ORDER: Always last, afterwards destructor HAS TO BE CALLED
	m_pPropertyProvider->AddPropertyListener(this);
}

// Destructor.
GenericPropertyBinder::~GenericPropertyBinder()
{
	disconnect(this, SLOT(updateProperties()));
	m_pPropertyProvider->RemovePropertyListener(this);
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

		// Update components
		for (int row = m_componentStartIdx; row < m_componentEndIdx; ++row)
		{
			uint4 componentIdx = static_cast<uint4>(row - m_componentStartIdx);

			QStandardItem *pComponentItem = m_pParentItem->child(row, 0);

			GenericPropertyBinder *pChildBinder = pComponentItem->data().value<GenericPropertyBinder*>();
			const beCore::PropertyProvider *pPreviousChildProvider = (pChildBinder) ? pChildBinder->propertyProvider() : nullptr;
			
			beCore::PropertyProvider *pChildProvider = m_pComponent->GetPropertyProvider(componentIdx);

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

				beCore::ReflectedComponent *pChildComponent = m_pComponent->GetReflectedComponent(componentIdx);

				// Replace with sub-tree of new child component
				pChildBinder = new GenericPropertyBinder(pChildProvider, pChildComponent, m_pDocument, m_pTree, pComponentItem, this);
				checkedConnect(this, SIGNAL(propagateUpdateProperties()), pChildBinder, SLOT(updateProperties()));

				pComponentItem->setData( QVariant::fromValue(pChildBinder) );
			}

			// TODO: Value changes
		}

		m_bPropertiesChanged = false;
	}

	Q_EMIT propagateUpdateProperties();
}

// Called when the given propery might have changed.
void GenericPropertyBinder::PropertyChanged(const beCore::PropertyProvider &provider)
{
	m_bPropertiesChanged = true;
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
			m_pPropertyProvider->WriteProperty(propertyID, beCore::PropertyDataVisitor(propertyWriter), false);
			
			// Capture new property value
			command->capture();
			m_pDocument->undoStack()->push( command.detach() );
		}
	}
}

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
		}
		// Component
		else if (m_componentStartIdx <= row && row < m_componentEndIdx)
		{
			uint4 componentIdx = static_cast<uint4>( row - m_componentStartIdx );

			beCore::Exchange::utf8_string componentType = m_pComponent->GetComponentType(componentIdx);
			
			const beCore::ComponentReflector *pReflector = beCore::GetComponentTypes().GetReflector(componentType);

			if (pReflector)
			{
				// Create editor panel (opaque for focus, mouse & eyes)
				QFrame *editor = new QFrame(parent);
				editor->setAttribute(Qt::WA_NoMousePropagation);
				editor->setFocusPolicy(Qt::StrongFocus);
				editor->setAutoFillBackground(true);
				editor->setFrameShape(QFrame::StyledPanel);

				QVBoxLayout *layout = new QVBoxLayout(editor);

				// Create component editor
				ComponentSelectorWidget *componentSelector =  new ComponentSelectorWidget(pReflector, editor);
				layout->addWidget(componentSelector);

				editor->setLayout(layout);
				editor->adjustSize();
				pEditor = editor;
			}
			else
				QMessageBox::critical(nullptr,
						tr("Unknown component type"),
						tr("Component type '%1' is unknown.").arg( toQt(componentType) )
					);

			bHandled = true;
		}
	}
}

// Data currently being edited has changed.
void GenericPropertyBinder::updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled)
{
	if (index.parent() == m_pParentItem->index())
	{
		int row = index.row();

		// Component editors are never updated after they've been opened
		if (m_componentStartIdx <= row && row < m_componentEndIdx)
			bHandled = true;
	}
}

// Editor data is requested.
void GenericPropertyBinder::updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled)
{
	if (index.parent() == m_pParentItem->index())
	{
		int row = index.row();

		if (m_componentStartIdx <= row && row < m_componentEndIdx)
		{
			uint4 componentIdx = static_cast<uint4>(row - m_componentStartIdx);

			ComponentSelectorWidget *pComponentSelector = editor->findChild<ComponentSelectorWidget*>();

			if (pComponentSelector)
			{
				try
				{
					// Capture current component
					lean::scoped_ptr<SetComponentCommand> command(
							new SetComponentCommand(
								m_pComponent, componentIdx,
								pComponentSelector->acquireComponent(*m_pDocument)
							)
						);

					// Set new component
					m_pDocument->undoStack()->push( command.detach() );
				}
				catch (...)
				{
					exceptionToMessageBox(
							GenericPropertyBinder::tr("Error setting component"),
							GenericPropertyBinder::tr("An unexpected error occurred while setting component '%1'.").arg( toQt(m_pComponent->GetComponentName(componentIdx)) )
						);
				}
			}

			bHandled = true;
		}
	}
}

// Editor to be relocated.
void GenericPropertyBinder::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled)
{
	if (index.parent() == m_pParentItem->index())
	{
		int row = index.row();

		if (m_componentStartIdx <= row && row < m_componentEndIdx)
		{
			// Use almost entire with (leave some small space for click escape)
			int itemWidth = option.rect.width();
			int width = (3 * editor->parentWidget()->width() + itemWidth) / 4;

			// Put editor below property
			editor->move(option.rect.left() - (width - itemWidth), option.rect.bottom());
			editor->resize(width, editor->height());
			bHandled = true;
		}
	}
}

// Sets the given tree view up for property editing.
void GenericPropertyBinder::setupTree(QTreeView &view)
{
	ItemDelegateEx *pItemDelegate = qobject_cast<ItemDelegateEx*>( view.itemDelegate() );
	
	// This binder needs the enhanced item delegate
	if (!pItemDelegate)
		view.setItemDelegate( new ItemDelegateEx(&view) );
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
