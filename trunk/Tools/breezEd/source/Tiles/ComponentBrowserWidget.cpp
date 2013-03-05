#include "stdafx.h"
#include "Tiles/ComponentBrowserWidget.h"

#include <QtCore/QSignalMapper>

#include "Documents/SceneDocument.h"
#include "Binders/ComponentListBinder.h"

#include "Widgets/ComponentPickerFactory.h"
#include "Plugins/FactoryManager.h"

#include <beCore/beParameterSet.h>
#include <beCore/beComponentReflector.h>
#include <beCore/beReflectionTypes.h>

#include "Utility/InlineEditor.h"
#include "Utility/WidgetSupport.h"
#include "Utility/IconDockStyle.h"
#include "Utility/Strings.h"
#include "Utility/Checked.h"
#include "Utility/UI.h"

#include "Editor.h"
#include "Windows/MainWindow.h"

#include "Utility/CollectionListWidget.h"

#include <lean/logging/errors.h>


namespace
{

QString nameByComponent(const bec::ComponentTypeDesc &desc, const lean::any &component)
{
	return toQt( LEAN_ASSERT_NOT_NULL(desc.Reflector)->GetInfo(component).Name );
}

QString fileByComponent(const bec::ComponentTypeDesc &desc, const lean::any &component)
{
	return toQt( LEAN_ASSERT_NOT_NULL(desc.Reflector)->GetInfo(component).File );
}

lean::cloneable_obj<lean::any, lean::ptr_sem> componentByName(SceneDocument &document, const bec::ComponentTypeDesc &desc, const QString &name)
{
	lean::cloneable_obj<lean::any, lean::ptr_sem> pComponent(nullptr);

	if (!name.isEmpty())
		try
		{
			const bec::ComponentReflector *reflector = LEAN_ASSERT_NOT_NULL( desc.Reflector );
			bec::ParameterSet parameters = document.getSerializationParameters();
			pComponent = reflector->GetComponentByName(toUtf8(name), parameters);
		}
		catch (...)
		{
			exceptionToMessageBox(
					ComponentBrowserWidget::tr("Error retrieving component"),
					ComponentBrowserWidget::tr("An error occurred while trying to retrieve component '%1'.").arg(name)
				);
		}
	return pComponent;
}

void adaptUI(Ui::ComponentBrowserWidget &ui, const beCore::ComponentReflector &reflector, Editor *editor,
			 const lean::any *pPrototype, bool bInitialize = false)
{
	uint4 componentFlags = reflector.GetComponentFlags();

	bool bCreatable = componentFlags & bec::ComponentFlags::Creatable;
	bool bCloneable = componentFlags & bec::ComponentFlags::Cloneable;
	bool bFiled = componentFlags & bec::ComponentFlags::Filed;

	bool bFileMode = ui.importButton->isChecked();
	if (bInitialize)
	{
		bFileMode = bFiled && (bFileMode || !bCreatable);
		ui.importButton->setChecked(bFileMode);
	}
	bool bCreateMode = !bFileMode;
	
	if (bInitialize)
	{
		ui.newButton->setVisible(bCreatable);
		ui.importButton->setVisible(bFiled);
	}

	ui.nameGroupBox->setEnabled(bCreateMode && bCreatable);
	ui.nameGroupBox->setVisible(bCreateMode);

	ui.fileGroupBox->setEnabled(bFileMode && bFiled);
	ui.fileGroupBox->setVisible(bFileMode);

	if (bInitialize)
	{
		ui.cloneCheckBox->setEnabled(bCloneable);
		ui.cloneCheckBox->setVisible(bCloneable);
		ui.cloneCheckBox->setChecked(bCloneable);
	}

	// TODO: Also move info update here
	beCore::Parameters defaultValues;
	beCore::ComponentParameters parameters;

	if (bCreateMode && bCreatable)
	{
		parameters = reflector.GetCreationParameters();
		if (pPrototype)
			reflector.GetCreationInfo(*pPrototype, defaultValues);
	}
	else if (bFileMode && bFiled)
	{
		parameters = reflector.GetFileParameters(""); // TODO
		if (pPrototype)
			reflector.GetFileInfo(*pPrototype, defaultValues);
	}

	{
		lean::scoped_ptr<ComponentParameterWidget> parameterWidget( new ComponentParameterWidget(
				parameters, defaultValues, bCreateMode && pPrototype, editor, ui.parameterBox
			) );
		ui.parameterBox->layout()->addWidget(parameterWidget);
		delete ui.parameterWidget;
		ui.parameterWidget = parameterWidget.detach();
	}

	ui.parameterBox->setVisible(ui.parameterWidget->hasParameters());
}

lean::cloneable_obj<lean::any, lean::ptr_sem> createComponent(Ui::ComponentBrowserWidget &ui, const beCore::ComponentReflector &reflector, SceneDocument &document,
															  const lean::any *pPrototype, const lean::any *pReplace)
{
	return reflector.CreateComponent( toUtf8Range(ui.nameEdit->text()),
			ui.parameterWidget->getParameters(), document.getSerializationParameters(),
			ui.cloneCheckBox->isChecked() ? pPrototype : nullptr,
			pReplace
		);
}

lean::cloneable_obj<lean::any, lean::ptr_sem> loadComponent(Ui::ComponentBrowserWidget &ui, const beCore::ComponentReflector &reflector, SceneDocument &document)
{
	return reflector.GetComponentByFile( toUtf8Range(ui.browseWidget->path()),
			ui.parameterWidget->getParameters(), document.getSerializationParameters()
		);
}

} // namespace

// Constructor.
ComponentBrowserWidget::ComponentBrowserWidget(const beCore::ComponentTypeDesc *type, Editor *editor, QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags),
	m_editor( LEAN_ASSERT_NOT_NULL(editor) ),
	m_pDocument(),
	m_type(type)
{
	LEAN_ASSERT(type && type->Reflector);

	ui.setupUi(this);
	ui.listView->sortByColumn(0, Qt::AscendingOrder);
	ui.splitter->setSizes( QList<int>() << 2000 << 100 );
	setWindowTitle(toQt(m_type->Name) + " Browser");

	adaptUI(ui, *m_type->Reflector, m_editor, nullptr, true);

	connect(ui.syncButton, &QPushButton::toggled, this, &ComponentBrowserWidget::syncChanged, Qt::QueuedConnection);
	connect(ui.createButton, &QPushButton::clicked, this, &ComponentBrowserWidget::newComponent, Qt::QueuedConnection);
	connect(ui.replaceButton, &QPushButton::clicked, this, &ComponentBrowserWidget::replaceComponent, Qt::QueuedConnection);
	connect(ui.loadButton, &QPushButton::clicked, this, &ComponentBrowserWidget::loadComponent, Qt::QueuedConnection);

	connect(ui.importButton, &QPushButton::toggled, this, &ComponentBrowserWidget::toggleMode, Qt::QueuedConnection);
	connect(ui.browseWidget, &BrowseWidget::browse, this, &ComponentBrowserWidget::browse);
	connect(ui.browseWidget, &BrowseWidget::pathSelected, this, &ComponentBrowserWidget::adapt, Qt::QueuedConnection);
	
	connect(m_editor->mainWindow(), &MainWindow::focusChanged, this, &ComponentBrowserWidget::focusChanged);
}

// Destructor.
ComponentBrowserWidget::~ComponentBrowserWidget()
{
}

// Adds a new component.
void ComponentBrowserWidget::newComponent()
{
	if (!m_pListBinder)
		return;

	try
	{
		lean::cloneable_obj<lean::any, lean::ptr_sem> component = createComponent(this->ui, *m_type->Reflector, *m_pDocument, selectedComponent(), nullptr);
		LEAN_THROW_NULL(component.getptr());

		m_pListBinder->updateTree();
		selectComponent(component);
	}
	catch (...)
	{
		exceptionToMessageBox(
				tr("Error creating component"),
				tr("An unexpected error occurred while creating component '%1'.").arg( makeName(ui.nameEdit->text()) )
			);
	}
}

// Replaces the selected component.
void ComponentBrowserWidget::replaceComponent()
{
	if (!m_pListBinder)
		return;

	try
	{
		lean::cloneable_obj<lean::any, lean::ptr_sem> replace = selectedComponent();

		if (replace)
		{
			lean::cloneable_obj<lean::any, lean::ptr_sem> component = createComponent(this->ui, *m_type->Reflector, *m_pDocument, replace, replace);
			LEAN_THROW_NULL(component.getptr());

			m_pListBinder->updateTree();
			selectComponent(component);
		}
		else
			QMessageBox::information(nullptr, tr("Incomplete information"), tr("Select the component to be replaced."));
	}
	catch (...)
	{
		exceptionToMessageBox(
				tr("Error replacing component"),
				tr("An unexpected error occurred while creating component '%1'.").arg( makeName(ui.nameEdit->text()) )
			);
	}
}

// Loads a new component.
void ComponentBrowserWidget::loadComponent()
{
	if (!m_pListBinder)
		return;

	try
	{
		lean::cloneable_obj<lean::any, lean::ptr_sem> component = ::loadComponent(this->ui, *m_type->Reflector, *m_pDocument);
		LEAN_THROW_NULL(component.getptr());

		m_pListBinder->updateTree();
		selectComponent(component);
	}
	catch (...)
	{
		exceptionToMessageBox(
				tr("Error loading component"),
				tr("An unexpected error occurred while loading component '%1'.").arg( ui.browseWidget->path() )
			);
	}
}

// Browses for a component file.
void ComponentBrowserWidget::browse()
{
	const ComponentPickerFactory &componentPickerFactory = *LEAN_ASSERT_NOT_NULL(
			getComponentPickerFactories().getFactory( toQt(m_type->Name) ) // TODO: Not by name?
		);

	// Browse for component using type-specific functionality
	QPoint pos = QCursor::pos();
	QString file = componentPickerFactory.browseForComponent(*m_type->Reflector, ui.browseWidget->path(), *m_editor, this);
	QCursor::setPos(pos);

	if (!file.isEmpty())
		ui.browseWidget->setPath(file);
}

// Toggles the mode.
void ComponentBrowserWidget::toggleMode()
{
	adapt();
}

// Reacts to file or mode changes.
void ComponentBrowserWidget::adapt()
{
	adaptUI(ui, *LEAN_ASSERT_NOT_NULL(m_type->Reflector), m_editor, selectedComponent());
}

// Focus changed.
void ComponentBrowserWidget::focusChanged(QWidget *prev, QWidget *next)
{
	if (isChildOf(prev, this) && next && !next->isModal() && !isChildOf(next, this))
		Q_EMIT focusLost(this);
}

// Sets the browser color.
void ComponentBrowserWidget::setColor(QColor color)
{
	QColor lighter( 255 - (255 - color.red()) / 3, 255 - (255 - color.green()) / 3, 255 - (255 - color.blue()) / 3 );
	QPalette palette = ui.listView->palette();
	palette.setColor(QPalette::Base, lighter);
	palette.setColor(QPalette::AlternateBase, color);
	ui.listView->setPalette(palette);
}

// Gets the browser color.
QColor ComponentBrowserWidget::color() const
{
	return ui.listView->palette().alternateBase().color();
}

// Checks whether the browser is in use.
bool ComponentBrowserWidget::inUse() const
{
	return this->receivers(SIGNAL(componentSelected(const lean::any&))) > 0;
}

// Sets the current document.
void ComponentBrowserWidget::setDocument(AbstractDocument *pDocument)
{
	if (pDocument != m_pDocument)
	{
		if (m_pDocument)
			disconnect(m_pDocument, nullptr, this, nullptr);

		m_pListBinder = nullptr;

		m_pDocument = qobject_cast<SceneDocument*>(pDocument);

		if (m_pDocument)
		{
			m_pListBinder = new ComponentListBinder(m_type, m_pDocument, windowIcon(), ui.listView, nullptr);
			connect(m_pListBinder, &ComponentListBinder::nameChanged, this, &ComponentBrowserWidget::nameChanged);
			connect(m_pListBinder, &ComponentListBinder::selectionChanged, this, &ComponentBrowserWidget::selectionChanged);
			connect(m_pListBinder, &ComponentListBinder::selectionChanged, this, &ComponentBrowserWidget::syncUI);
			connect(m_pDocument, &SceneDocument::postCommit, this, &ComponentBrowserWidget::processChanges);
		}
		
		ui.newGroupBox->setEnabled(m_pDocument != nullptr);
	}
}

/// Gets the selected component.
lean::cloneable_obj<lean::any, lean::ptr_sem> ComponentBrowserWidget::selectedComponent() const
{
	return (m_pListBinder)
		? componentByName(*m_pDocument, *m_type, m_pListBinder->selectedName())
		: lean::cloneable_obj<lean::any, lean::ptr_sem>(nullptr);
}

// Selects the given component.
void ComponentBrowserWidget::selectComponent(const lean::any *pComponent)
{
	if (m_pListBinder && pComponent)
		m_pListBinder->selectName( nameByComponent(*m_type, *pComponent) );
}

// Moves the cursor to the current item.
void ComponentBrowserWidget::moveToCurrent()
{
	if (m_pListBinder)
		m_pListBinder->moveToCurrent();
}

// Processes component management changes.
void ComponentBrowserWidget::processChanges()
{
	if (m_pDocument && m_pDocument->graphicsResources()->Monitor()->Management.HasChanged(m_type->Type))
		m_pListBinder->updateTree();
}

// Name changed.
void ComponentBrowserWidget::nameChanged(const QString &oldName, const QString &newName, bool &bSuccess)
{
	lean::cloneable_obj<lean::any, lean::ptr_sem> pComponent = componentByName(*m_pDocument, *m_type, oldName);

	if (!pComponent)
	{
		QMessageBox::critical( nullptr,
				tr("Unknown component."),
				tr("The component '%1' is no longer known and cannot be renamed.").arg(oldName)
			);
		return;
	}

	try
	{
		m_type->Reflector->SetName(*pComponent, toUtf8(newName));
		bSuccess = true;
	}
	catch (...)
	{
		exceptionToMessageBox(
				tr("Error renaming component."),
				tr("The component '%1' could not be renamed to '%2'.").arg(oldName, newName)
			);
	}
}

// Synchonizes the creation UI with the given component.
void ComponentBrowserWidget::syncUI(const QString &componentName)
{
	// Update synchronized creation UI
	if (ui.syncButton->isChecked())
	{
		if (!componentName.isEmpty())
			ui.nameEdit->setText(componentName);

		if (lean::cloneable_obj<lean::any, lean::ptr_sem> component = componentByName(*m_pDocument, *m_type, componentName))
		{
			ui.browseWidget->setPath(fileByComponent(*m_type, *component));
//			adaptUI(ui, *m_type->Reflector, m_editor, component);
		}
	}
}

// Synchronization changed.
void ComponentBrowserWidget::syncChanged()
{
	if (ui.syncButton->isChecked() && m_pListBinder)
		syncUI(m_pListBinder->selectedName());
}

// Selection changed.
void ComponentBrowserWidget::selectionChanged(const QString &componentName)
{
	if (!componentName.isEmpty())
	{
		lean::cloneable_obj<lean::any, lean::ptr_sem> pComponent = componentByName(*m_pDocument, *m_type, componentName);

		if (pComponent)
			// A component has been selected
			Q_EMIT componentSelected(pComponent, componentName);
	}
}

// Constructor.
ComponentSelectorWidget::ComponentSelectorWidget(ComponentBrowserWidget *browser, QWidget *pParent)
	: QLineEdit(pParent),
	m_editor(browser->editor()),
	m_type(browser->componentType()),
	m_pComponent(nullptr),
	m_pBrowser(nullptr),
	m_bJustEnteredByMouse(false)
{
	this->setReadOnly(true);
	linkToBrowser(browser);
}

// Constructor.
ComponentSelectorWidget::ComponentSelectorWidget(const beCore::ComponentTypeDesc *type, Editor *editor, QWidget *pParent)
	: QLineEdit(pParent),
	m_editor(editor),
	m_type(type),
	m_pComponent(nullptr),
	m_pBrowser(nullptr),
	m_bJustEnteredByMouse(false)
{
	this->setReadOnly(true);
}

// Destructor.
ComponentSelectorWidget::~ComponentSelectorWidget()
{
}

// Links this selector to the given component browser.
void ComponentSelectorWidget::linkToBrowser(ComponentBrowserWidget *pBrowser, bool bVolatile)
{
	bool bUnlinked = m_pBrowser && !pBrowser;

	if (m_pBrowser)
	{
		disconnect(m_pBrowser, nullptr, this, nullptr);
		disconnect(this, &QLineEdit::returnPressed, this, nullptr);
	}

	m_pBrowser = pBrowser;
	
	if (m_pBrowser)
	{
		QPalette palette = this->palette();
		palette.setColor(QPalette::Base, m_pBrowser->color());
		this->setPalette(palette);

		connect(m_pBrowser, &ComponentBrowserWidget::componentSelected, this, &ComponentSelectorWidget::setComponent);
		
		if (bVolatile)
		{
			// Volatile links are dissolved on browser focus loss and return key
			// NOTE: unlink() issues selectionFinished()
			connect(m_pBrowser, &ComponentBrowserWidget::focusLost, this, &ComponentSelectorWidget::unlink);
			connect(this, &QLineEdit::returnPressed, this, &ComponentSelectorWidget::unlink);
		}
		else
		{
			// Non-volatile links persist even when the selection has been accepted
			connect(m_pBrowser, &ComponentBrowserWidget::focusLost, this, &ComponentSelectorWidget::selectionFinished);
			connect(this, &QLineEdit::returnPressed, this, &ComponentSelectorWidget::selectionFinished);
		}
	}
	else
		this->setPalette(QApplication::palette(this));

	if (bUnlinked)
		Q_EMIT selectionFinished();
}

// Links this selector to a component browser.
void ComponentSelectorWidget::link()
{
	if (!m_pBrowser)
		linkToBrowser( retrieveBrowser(m_type, m_editor->mainWindow(), this), true );
}

// Unlinks this selector from its current browser.
void ComponentSelectorWidget::unlink()
{
	linkToBrowser(nullptr);
}

// Toggles linked state.
void ComponentSelectorWidget::toggleLinked()
{
	if (!m_pBrowser)
		link();
	else
		unlink();
}

void ComponentSelectorWidget::focusInEvent(QFocusEvent *event)
{
	QLineEdit::focusInEvent(event);

	m_bJustEnteredByMouse = (event->reason() == Qt::MouseFocusReason);

	if (m_bJustEnteredByMouse)
		toggleLinked();
	else
		link();
}

void ComponentSelectorWidget::mouseReleaseEvent(QMouseEvent *event)
{
	QLineEdit::mouseReleaseEvent(event);

	if (!m_bJustEnteredByMouse)
		toggleLinked();
	m_bJustEnteredByMouse = false;
}

// Sets the given component.
void ComponentSelectorWidget::setComponent(const lean::any *pComponent, const QString &componentName)
{
	if (pComponent != m_pComponent)
	{
		m_pComponent = pComponent;
		setText(componentName);

		Q_EMIT componentChanged(m_pComponent);
	}
}

ComponentCollectionWidget::ComponentCollectionWidget(const beCore::ComponentTypeDesc *type, Editor *editor, QWidget *pParent)
	: CollectionListWidget(pParent),
	m_editor(editor),
	m_type(type)
{
	connect(this, &CollectionListWidget::itemAdded, this, &ComponentCollectionWidget::prepareItem, Qt::DirectConnection);
	connect(this, &CollectionListWidget::itemDeleted, this, &ComponentCollectionWidget::finalizeItem, Qt::DirectConnection);

	connect(itemDelegateEx(), &ItemDelegateEx::startEditing, this, &ComponentCollectionWidget::startEditing, Qt::DirectConnection);
	connect(itemDelegateEx(), &ItemDelegateEx::updateEditor, this, &ComponentCollectionWidget::updateEditor, Qt::DirectConnection);
	connect(itemDelegateEx(), &ItemDelegateEx::updateData, this, &ComponentCollectionWidget::updateData, Qt::DirectConnection);
	connect(itemDelegateEx(), &ItemDelegateEx::editorGeometryChanged, this, &ComponentCollectionWidget::updateEditorGeometry, Qt::DirectConnection);
}

// Destructor.
ComponentCollectionWidget::~ComponentCollectionWidget()
{
}

template <class T>
inline QVariant variantPointer(T *p)
{
	return QVariant::fromValue( reinterpret_cast<uintptr_t>(p) );
}

template <class T>
inline T* variantPointer(const QVariant &v)
{
	return reinterpret_cast<T*>( v.value<uintptr_t>() );
}

// Adds the given component.
void ComponentCollectionWidget::addComponent(const lean::any *pComponent, const QString &componentName)
{
	QStandardItem *item = this->addItem();
	item->setText(componentName);
	*variantPointer<components_t::value_type>( item->data(Qt::UserRole) ) = pComponent;
}

// Gets the components.
ComponentCollectionWidget::ComponentVector ComponentCollectionWidget::components() const
{
	ComponentVector components;
	QList<QStandardItem*> items = this->items();
	components.reserve(items.size());

	Q_FOREACH (QStandardItem *item, items)
		components.push_back( *LEAN_ASSERT_NOT_NULL( variantPointer<components_t::value_type>( item->data(Qt::UserRole) ) ) );

	return components;
}

// Item added.
void ComponentCollectionWidget::prepareItem(QStandardItem *item)
{
	m_components.push_back(components_t::value_type(nullptr));
	item->setText(tr("<unset>"));
	item->setData(variantPointer(&m_components.back()), Qt::UserRole);
}

// Item removed.
void ComponentCollectionWidget::finalizeItem(QStandardItem *item)
{
	components_t::value_type *itemData = LEAN_ASSERT_NOT_NULL( variantPointer<components_t::value_type>( item->data(Qt::UserRole) ) );

	for (components_t::iterator it = m_components.begin(); it != m_components.end(); ++it)
		if (&*it == itemData)
		{
			m_components.erase(it);
			return;
		}
}

// Editing has started.
void ComponentCollectionWidget::startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled)
{
	QStandardItem *item = this->model()->itemFromIndex(index);

	lean::scoped_ptr<ComponentSelectorWidget> selector( new ComponentSelectorWidget(m_type, m_editor) );
	selector->setComponent( variantPointer<components_t::value_type>( item->data(Qt::UserRole) )->getptr(), item->text() );

	connect(selector, &ComponentSelectorWidget::selectionFinished, this, &ComponentCollectionWidget::selectionFinished);

	lean::scoped_ptr<InlineEditor> editor( new InlineEditor(false, parent) );
	editor->setWidget(selector.detach());

	pEditor = editor.detach();
	bHandled = true;
}

// Data currently being edited has changed.
void ComponentCollectionWidget::updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled)
{
	bHandled = true;
}

// Editor data is requested.
void ComponentCollectionWidget::updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled)
{
	QStandardItem *item = this->model()->itemFromIndex(index);
	ComponentSelectorWidget *selector = LEAN_ASSERT_NOT_NULL( qobject_cast<InlineEditor*>(editor) )->widget<ComponentSelectorWidget>();
	
	item->setText(selector->text());
	*variantPointer<components_t::value_type>( item->data(Qt::UserRole) ) = selector->component();
	bHandled = true;
}

// Editor to be relocated.
void ComponentCollectionWidget::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled)
{
	editor->setGeometry(option.rect);
	bHandled = true;
}

// Commits data and closes the editor.
void ComponentCollectionWidget::selectionFinished()
{
	ComponentSelectorWidget *selector = LEAN_ASSERT_NOT_NULL( qobject_cast<ComponentSelectorWidget*>(sender()) );
	Q_EMIT itemDelegateEx()->commitData(selector);
	Q_EMIT itemDelegateEx()->closeEditor(selector);
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"
#include "Docking/DockContainer.h"

// Opens a component browser for the given component type.
ComponentBrowserWidget* openBrowser(const QString &componentTypeName, MainWindow *mainWindow, QWidget *pAvoidOverlap, bool bHidden)
{
	const bec::ComponentTypeDesc *componentTypeDesc = bec::GetComponentTypes().GetDesc(toUtf8(componentTypeName));

	if (!componentTypeDesc || !componentTypeDesc->Reflector)
	{
		QMessageBox::critical( nullptr,
				ComponentBrowserWidget::tr("Unknown component type"),
				ComponentBrowserWidget::tr("Cannot open '%1' component browser. Component type unknown or insufficiently reflected.").arg(componentTypeName)
			);
		return nullptr;
	}

	QString dockWidgetNameBase = "ComponentBrowserWidget_" + componentTypeName;
	QString dockWidgetName = dockWidgetNameBase;
	uint4 dockWidgetNameCounter = 0;

	// Find unique widget name
	while (mainWindow->findChild<QDockWidget*>(dockWidgetName))
		dockWidgetName = dockWidgetNameBase + ++dockWidgetNameCounter;

/*	lean::scoped_ptr<QDockWidget> dock( new QDockWidget(mainWindow) );
	dock->setObjectName(dockWidgetName);
//	dock->setAttribute(Qt::WA_DeleteOnClose); TODO: Close all but last?
	dock->setStyle( new IconDockStyle(dock, dock->style()) );

	ComponentBrowserWidget *browser = new ComponentBrowserWidget(componentTypeDesc, mainWindow->editor(), dock);
	dock->setWidget(browser);
	dock->setWindowTitle(browser->windowTitle());
	dock->setWindowIcon(browser->windowIcon());
*/
	lean::scoped_ptr<ComponentBrowserWidget> widget( new ComponentBrowserWidget(componentTypeDesc, mainWindow->editor()) );
	widget->setObjectName(dockWidgetName);
	lean::scoped_ptr<DockWidget> dock( DockWidget::wrap(widget) );
	dock->setAttribute(Qt::WA_DeleteOnClose);
	widget->setColor( QColor(200 + qrand() % 56, 200 + qrand() % 56, 200 + qrand() % 56) );

	if (bHidden)
		dock->hide();
	
	mainWindow->dock()->addDock(dock, DockPlacement::Emplace, DockOrientation::Horizontal, DockSide::Before); 
	
	if (pAvoidOverlap && mainWindow->dock()->sameGroup(dock, pAvoidOverlap))
		mainWindow->dock()->addDock(dock, DockPlacement::Independent, DockOrientation::Horizontal, DockSide::After, pAvoidOverlap, DockCompany::JoinNeighbor);
	
	if (!bHidden)
		mainWindow->dock()->raiseDock(dock);

/*	// Invisible by default
	mainWindow->addDockWidgetTabified(Qt::LeftDockWidgetArea, dock);
	
	dock->show();
	dock->raise();
	
	// TODO: BROKEN in Qt 5.0.0
	if (QDockWidget *overlapping = mainWindow->overlappingDockWidget(dock, pAvoidOverlap))
		mainWindow->splitDockWidget(overlapping, dock, Qt::Horizontal);
*/
	QObject::connect(mainWindow, &MainWindow::documentChanged, widget, &ComponentBrowserWidget::setDocument);

	widget->setDocument(mainWindow->activeDocument());

	dock.detach();
	return widget.detach();
}

// Opens or retrieves an unused open component browser for the given component type.
ComponentBrowserWidget* retrieveBrowser(const bec::ComponentTypeDesc *componentType, MainWindow *mainWindow, QWidget *pAvoidOverlap, bool *pLayoutChanged)
{
	if (pLayoutChanged)
		*pLayoutChanged = false;

	Q_FOREACH(ComponentBrowserWidget *browser, mainWindow->findChildren<ComponentBrowserWidget*>())
		// Find matching free browser
		if (browser->componentType() == componentType && !browser->inUse())
		{
			if (DockWidget *dock = findParent<DockWidget*>(browser))
			{
				// Split overlapping browsers
				if (pAvoidOverlap && mainWindow->dock()->sameGroup(dock, pAvoidOverlap))
					mainWindow->dock()->addDock(dock, DockPlacement::Independent, DockOrientation::Horizontal, DockSide::After, pAvoidOverlap, DockCompany::JoinNeighbor);

				if (pLayoutChanged)
					*pLayoutChanged = !browser->isVisible();

				dock->showAndRaise();
			}
			else
			{
				if (pLayoutChanged)
					*pLayoutChanged = !browser->isVisible();

				browser->show();
				browser->raise();
			}

/*			QWidget *pDock = browser; // findParent<QDockWidget*>(browser);
	
			if (pDock)
				if (QDockWidget *overlapping = mainWindow->overlappingDockWidget(pDock, pAvoidOverlap))
				{
//					continue;
					// TODO: BROKEN in Qt 5.0.0
					mainWindow->splitDockWidget(pDock, overlapping, Qt::Horizontal);
					if (pLayoutChanged) *pLayoutChanged = true;
				}

			if (pDock)
			{
				if (pLayoutChanged)
					*pLayoutChanged = !pDock->isVisible();

				pDock->show();
				pDock->raise();
			}
*/

			return browser;
		}

	if (pLayoutChanged)
		*pLayoutChanged = true;

	return openBrowser(toQt(componentType->Name), mainWindow, pAvoidOverlap);
}

#include "Docking/DockContainer.h"

namespace
{

/// Plugin class.
struct ComponentBrowserWidgetPlugin : public AbstractPlugin<MainWindow*>, public QObject
{
	/// Constructor.
	ComponentBrowserWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~ComponentBrowserWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *mainWindow) const
	{
		{
			lean::scoped_ptr<QToolButton> componentButton( new QToolButton() );
			componentButton->setIcon(mainWindow->widgets().actionComponent_Browser->icon());
			componentButton->setToolTip(mainWindow->widgets().actionComponent_Browser->toolTip());
			componentButton->setMenu(mainWindow->widgets().menuComponents);
			componentButton->setPopupMode(QToolButton::InstantPopup);
			mainWindow->widgets().assetToolBar->addWidget(componentButton.detach());
		}

		lean::scoped_ptr<QSignalMapper> signalMapper( new QSignalMapper(mainWindow) );
		bec::ComponentTypes::TypeDescs componentTypeDescs = bec::GetComponentTypes().GetDescs();

		for (const bec::ComponentTypeDesc *const *it = &componentTypeDescs[0], *const* itEnd = &componentTypeDescs[componentTypeDescs.size()]; it < itEnd; ++it)
		{
			const bec::ComponentTypeDesc &componentTypeDesc = **it;
			
			// Ignore unreflected types
			if (!componentTypeDesc.Reflector)
				continue;

			QString componentType = toQt(componentTypeDesc.Name);
			lean::scoped_ptr<QAction> componentBrowserAction( new QAction(componentType, mainWindow->widgets().menuComponents) );
			signalMapper->setMapping(componentBrowserAction, componentType);
			connect(componentBrowserAction, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
			
			mainWindow->widgets().menuComponents->addAction(componentBrowserAction.detach());
		}

		connect(signalMapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
				this, &ComponentBrowserWidgetPlugin::newBrowser);
		signalMapper.detach();
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }

	/// Opens a new browser widget.
	void newBrowser(const QString &componentType)
	{
		QSignalMapper *signalMapper = LEAN_ASSERT_NOT_NULL( qobject_cast<QSignalMapper*>(sender()) );
		MainWindow *mainWindow = LEAN_ASSERT_NOT_NULL( qobject_cast<MainWindow*>(signalMapper->parent()) );

		openBrowser(componentType, mainWindow);
	}
};

const ComponentBrowserWidgetPlugin ComponentBrowserWidgetPlugin;

} // namespace
