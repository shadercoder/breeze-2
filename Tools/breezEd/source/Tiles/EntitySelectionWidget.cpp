#include "stdafx.h"
#include "Tiles/EntitySelectionWidget.h"

#include "Editor.h"

#include "Documents/SceneDocument.h"

#include <beEntitySystem/beEntityGroup.h>
#include <beEntitySystem/beAsset.h>

#include "Utility/IconDockStyle.h"
#include "Utility/Strings.h"
#include "Utility/Files.h"
#include "Utility/Checked.h"

#include <lean/functional/algorithm.h>

namespace
{

/// Updates the widget from the given selection.
void updateFromSelection(Ui::EntitySelectionWidget &ui, const EntitySelectionWidget::entity_vector &selection)
{
	ui.selectionList->clear();

	// Loop over windows
	Q_FOREACH (const beEntitySystem::Entity *entity, selection)
		// Add entity to list
		ui.selectionList->addItem( makeName(toQt(entity->GetName())) );

	ui.selectionList->selectAll();
}

/// Builds a list of all selected entities.
beEntitySystem::EntityGroup selectedEntities(Ui::EntitySelectionWidget &ui, const EntitySelectionWidget::entity_vector &selection)
{
	beEntitySystem::EntityGroup selectedEntities;

	QList<QListWidgetItem*> selectedItems = ui.selectionList->selectedItems();

	Q_FOREACH (QListWidgetItem *selectedItem, selectedItems)
	{
		int index = ui.selectionList->row(selectedItem);

		if (0 <= index && index < selection.size())
			selectedEntities.AddEntity(selection[index]);
	}

	return selectedEntities;
}

} // namespace

// Constructor.
EntitySelectionWidget::EntitySelectionWidget(Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pDocument()
{
	ui.setupUi(this);

	updateFromSelection(ui, m_selection);
	selectionChanged();

	checkedConnect(ui.selectionList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));

	checkedConnect(ui.assetButton, SIGNAL(clicked()), this, SLOT(toAsset()));
	checkedConnect(ui.deselectButton, SIGNAL(clicked()), this, SLOT(deselect()));
}

// Destructor.
EntitySelectionWidget::~EntitySelectionWidget()
{
}

// Sets the active selection of the given document.
void EntitySelectionWidget::setActiveSelection(SceneDocument *pDocument)
{
	if (pDocument)
	{
		const SceneDocument::EntityVector &selection = pDocument->selection();

		if (selection != m_selection)
		{
			m_selection = pDocument->selection();
			updateFromSelection(ui, m_selection);
		}
	}
	else if (!m_selection.empty())
	{
		m_selection.clear();
		updateFromSelection(ui, m_selection);
	}
}

// Sets the undo stack of the given document.
void EntitySelectionWidget::setDocument(AbstractDocument *pDocument)
{
	if (m_pDocument)
		disconnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(setActiveSelection(SceneDocument*)));

	m_pDocument = qobject_cast<SceneDocument*>(pDocument);

	if (m_pDocument)
		checkedConnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(setActiveSelection(SceneDocument*)));

	setActiveSelection(m_pDocument);
}

// Deselects currently selected entities.
void EntitySelectionWidget::deselect()
{
	if (m_pDocument && !m_selection.empty())
	{
		entity_vector selection = m_selection;

		QList<QListWidgetItem*> selectedItems = ui.selectionList->selectedItems();

		Q_FOREACH (QListWidgetItem *selectedItem, selectedItems)
		{
			int index = ui.selectionList->row(selectedItem);

			if (0 <= index && index < selection.size())
				selection[index] = nullptr;
		}

		lean::remove(selection, nullptr);

		m_pDocument->setSelection(selection);
	}
}

// Saves selected entities into an asset.
void EntitySelectionWidget::toAsset()
{
	if (!m_selection.empty())
	{
		QString file = saveFileDialog(
				this,
				EntitySelectionWidget::tr("Choose an asset file name"),
				*m_pEditor->settings(),
				"EntitySelectionWidget/assetPath",
				QString(),
				QString("%1 (*.asset.xml);;%2 (*.*)")
					.arg( EntitySelectionWidget::tr("Assets") )
					.arg( EntitySelectionWidget::tr("All Files") )
			);

		if (!file.isEmpty())
			beEntitySystem::SaveAsset(
					selectedEntities(ui, m_selection),
					toUtf8(file)
				);
	}
}

// Updates the ui.
void EntitySelectionWidget::selectionChanged()
{
	int selectedItemCount = ui.selectionList->selectedItems().count();

	ui.assetButton->setEnabled(selectedItemCount > 0);
	ui.groupButton->setEnabled(false); // TODO: selectedItemCount > 0
	ui.deselectButton->setEnabled(selectedItemCount > 0);
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"
#include "Docking/DockContainer.h"

namespace
{

/// Plugin class.
struct EntitySelectionWidgetPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	EntitySelectionWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~EntitySelectionWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *mainWindow) const
	{
/*		lean::scoped_ptr<QDockWidget> dock( new QDockWidget(mainWindow) );
		dock->setObjectName("EntitySelectionWidget");
		dock->setStyle( new IconDockStyle(dock, dock->style()) );

		dock->setWidget( new EntitySelectionWidget(mainWindow->editor(), dock) );
		dock->setWindowTitle(dock->widget()->windowTitle());
		dock->setWindowIcon(dock->widget()->windowIcon());
*/
		lean::scoped_ptr<EntitySelectionWidget> widget( new EntitySelectionWidget(mainWindow->editor()) );
		lean::scoped_ptr<DockWidget> dock( DockWidget::wrap(widget) );

		// Invisible by default
		dock->hide();
//		pMainWindow->addDockWidget(Qt::RightDockWidgetArea, dock);
		mainWindow->dock()->addDock(dock, DockPlacement::Emplace, DockOrientation::Horizontal);

		checkedConnect(mainWindow->widgets().actionEntity_Selection, SIGNAL(triggered()), dock, SLOT(showAndRaise()));
		checkedConnect(mainWindow, SIGNAL(documentChanged(AbstractDocument*)), widget, SLOT(setDocument(AbstractDocument*)));

		widget.detach();
		dock.detach();
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const EntitySelectionWidgetPlugin EntitySelectionWidgetPlugin;

} // namespace
