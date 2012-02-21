#include "stdafx.h"
#include "Modes/CodeModeState.h"

#include "Documents/CodeDocument.h"
#include "Editor.h"
#include "Windows/MainWindow.h"

#include "Utility/Checked.h"

// Constructor.
CodeModeState::CodeModeState(CodeDocument* pDocument, Editor *pEditor, QObject *pParent)
	: ModeState(pParent),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();

	// Enable undo / redo functionality on entry
	assignProperty(mainWindow.actionUndo, "enabled", false); // Set for proper revertion
	assignProperty(mainWindow.actionRedo, "enabled", false);
	assignProperty(mainWindow.actionUndo, "text", MainWindow::tr("Undo"));
	assignProperty(mainWindow.actionRedo, "text", MainWindow::tr("Redo"));
	addConnection(mainWindow.actionUndo, SIGNAL(triggered()), m_pDocument->textDocument(), SLOT(undo()));
	addConnection(mainWindow.actionRedo, SIGNAL(triggered()), m_pDocument->textDocument(), SLOT(redo()));
	addConnection(m_pDocument->textDocument(), SIGNAL(undoAvailable(bool)), mainWindow.actionUndo, SLOT(setEnabled(bool)));
	addConnection(m_pDocument->textDocument(), SIGNAL(redoAvailable(bool)), mainWindow.actionRedo, SLOT(setEnabled(bool)));
}

// Destructor.
CodeModeState::~CodeModeState()
{
}

// Called when the mode is entered.
void CodeModeState::onEntry(QEvent *pEvent)
{
	ModeState::onEntry(pEvent);

	// Initialize undo / redo
	undoRedoChanged();
}

// Called when the mode is exited.
void CodeModeState::onExit(QEvent *pEvent)
{
	ModeState::onExit(pEvent);
}

// Update undo / redo state.
void CodeModeState::undoRedoChanged()
{
	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();
	mainWindow.actionUndo->setEnabled(m_pDocument->textDocument()->isUndoAvailable());
	mainWindow.actionRedo->setEnabled(m_pDocument->textDocument()->isRedoAvailable());
}
