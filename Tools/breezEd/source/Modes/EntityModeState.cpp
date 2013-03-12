#include "stdafx.h"
#include "Modes/EntityModeState.h"

#include "Documents/SceneDocument.h"
#include "Editor.h"
#include "Windows/MainWindow.h"

#include "Interaction/SelectInteraction.h"
#include "Interaction/TranslateInteraction.h"
#include "Interaction/ScaleInteraction.h"
#include "Interaction/RotateInteraction.h"

#include "Operations/EntityOperations.h"

#include "Utility/Checked.h"

// Constructor.
EntityModeState::EntityModeState(SceneDocument* pDocument, Editor *pEditor, QObject *pParent)
	: ModeState(pParent),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),

	m_pSelectInteraction( new SelectInteraction(m_pDocument, this) ),
	m_pTranslateInteraction( new TranslateInteraction(m_pDocument, this) ),
	m_pRotateInteraction( new RotateInteraction(m_pDocument, this) ),
	m_pScaleInteraction( new ScaleInteraction(m_pDocument, this) )
{
	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();

	// Enable transform functionality on entry
	assignProperty(mainWindow.actionSelect, "enabled", true);
	addConnection(mainWindow.actionSelect, SIGNAL(triggered(bool)), this, SLOT(enableSelect(bool)));
	assignProperty(mainWindow.actionTranslate, "enabled", true);
	addConnection(mainWindow.actionTranslate, SIGNAL(triggered(bool)), this, SLOT(enableTranslate(bool)));
	assignProperty(mainWindow.actionRotate, "enabled", true);
	addConnection(mainWindow.actionRotate, SIGNAL(triggered(bool)), this, SLOT(enableRotate(bool)));
	assignProperty(mainWindow.actionScale, "enabled", true);
	addConnection(mainWindow.actionScale, SIGNAL(triggered(bool)), this, SLOT(enableScale(bool)));

	assignProperty(mainWindow.actionObject_Transform, "enabled", true);
	assignProperty(mainWindow.actionWorld_Transform, "enabled", true);

	assignProperty(mainWindow.actionDuplicate, "enabled", true);
	addConnection(mainWindow.actionDuplicate, SIGNAL(triggered(bool)), this, SLOT(duplicate()));
	assignProperty(mainWindow.actionRemove, "enabled", true);
	addConnection(mainWindow.actionRemove, SIGNAL(triggered(bool)), this, SLOT(remove()));
}

// Destructor.
EntityModeState::~EntityModeState()
{
}

// Called when the mode is entered.
void EntityModeState::onEntry(QEvent *pEvent)
{
	ModeState::onEntry(pEvent);

	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();
	
	enableSelect(mainWindow.actionSelect->isChecked());
	enableTranslate(mainWindow.actionTranslate->isChecked());
	enableRotate(mainWindow.actionRotate->isChecked());
	enableScale(mainWindow.actionScale->isChecked());
}

// Called when the mode is exited.
void EntityModeState::onExit(QEvent *pEvent)
{
	enableSelect(false);
	enableTranslate(false);
	enableRotate(false);
	enableScale(false);

	ModeState::onExit(pEvent);
}


// Enables the select tool.
void EntityModeState::enableSelect(bool bEnable)
{
	if (bEnable)
		m_pDocument->pushInteraction(m_pSelectInteraction);
	else
		m_pDocument->removeInteraction(m_pSelectInteraction);
}

// Enables the translate tool.
void EntityModeState::enableTranslate(bool bEnable)
{
	if (bEnable)
		m_pDocument->pushInteraction(m_pTranslateInteraction);
	else
		m_pDocument->removeInteraction(m_pTranslateInteraction);
}

// Enables the rotate tool.
void EntityModeState::enableRotate(bool bEnable)
{
	if (bEnable)
		m_pDocument->pushInteraction(m_pRotateInteraction);
	else
		m_pDocument->removeInteraction(m_pRotateInteraction);
}

// Enables the scale tool.
void EntityModeState::enableScale(bool bEnable)
{
	if (bEnable)
		m_pDocument->pushInteraction(m_pScaleInteraction);
	else
		m_pDocument->removeInteraction(m_pScaleInteraction);
}

// Clones the selection.
void EntityModeState::duplicate()
{
	DuplicateSelection(m_pDocument);
}

// Removes the selection.
void EntityModeState::remove()
{
	RemoveSelection(m_pDocument);
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"

#include "Utility/ExclusiveActionGroup.h"

namespace
{

/// Plugin class.
struct EntityPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	EntityPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~EntityPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *pMainWindow) const
	{
		// TODO: Remove?
/*		ExclusiveActionGroup *pToolGroup = new ExclusiveActionGroup(pMainWindow);
		pToolGroup->addAction(pMainWindow->widgets().actionSelect);
		pToolGroup->addAction(pMainWindow->widgets().actionTranslate);
		pToolGroup->addAction(pMainWindow->widgets().actionRotate);
		pToolGroup->addAction(pMainWindow->widgets().actionScale);
*/		
		// Default to select
		pMainWindow->widgets().actionSelect->setChecked(true);

		ExclusiveActionGroup *pFrameGroup = new ExclusiveActionGroup(pMainWindow);
		pFrameGroup->addAction(pMainWindow->widgets().actionObject_Transform);
		pFrameGroup->addAction(pMainWindow->widgets().actionWorld_Transform);

		// Default to local
		pMainWindow->widgets().actionObject_Transform->setChecked(true);
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const EntityPlugin EntityPlugin;

} // namespace
