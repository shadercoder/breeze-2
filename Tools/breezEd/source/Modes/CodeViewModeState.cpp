#include "stdafx.h"
#include "Modes/CodeViewModeState.h"

#include "Editor.h"
#include "Windows/MainWindow.h"
#include "Views/CodeView.h"

#include "Utility/Checked.h"

// Constructor.
CodeViewModeState::CodeViewModeState(CodeView* pView, Editor *pEditor, QObject *pParent)
	: ModeState(pParent),
	m_pView( LEAN_ASSERT_NOT_NULL(pView) ),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();

	// Enable clipboard functionality on entry
	assignProperty(mainWindow.actionCut, "enabled", false); // Set for proper revertion
	assignProperty(mainWindow.actionCopy, "enabled", false);
	assignProperty(mainWindow.actionPaste, "enabled", true);
	addConnection(m_pView->textEdit(), SIGNAL(copyAvailable(bool)), mainWindow.actionCut, SLOT(setEnabled(bool)));
	addConnection(m_pView->textEdit(), SIGNAL(copyAvailable(bool)), mainWindow.actionCopy, SLOT(setEnabled(bool)));
	addConnection(mainWindow.actionCut, SIGNAL(triggered()), m_pView->textEdit(), SLOT(cut()));
	addConnection(mainWindow.actionCopy, SIGNAL(triggered()), m_pView->textEdit(), SLOT(copy()));
	addConnection(mainWindow.actionPaste, SIGNAL(triggered()), m_pView->textEdit(), SLOT(paste()));
}

// Destructor.
CodeViewModeState::~CodeViewModeState()
{
}
