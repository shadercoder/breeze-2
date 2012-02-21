#include "stdafx.h"
#include "Modes/SceneModeState.h"

#include "Documents/SceneDocument.h"
#include "Editor.h"
#include "Windows/MainWindow.h"

#include "Utility/Checked.h"

// Constructor.
SceneModeState::SceneModeState(SceneDocument* pDocument, Editor *pEditor, QObject *pParent)
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
	addConnection(mainWindow.actionUndo, SIGNAL(triggered()), m_pDocument->undoStack(), SLOT(undo()));
	addConnection(mainWindow.actionRedo, SIGNAL(triggered()), m_pDocument->undoStack(), SLOT(redo()));
	addConnection(m_pDocument->undoStack(), SIGNAL(canUndoChanged(bool)), mainWindow.actionUndo, SLOT(setEnabled(bool)));
	addConnection(m_pDocument->undoStack(), SIGNAL(canRedoChanged(bool)), mainWindow.actionRedo, SLOT(setEnabled(bool)));
	addConnection(m_pDocument->undoStack(), SIGNAL(undoTextChanged(const QString&)), this, SLOT(undoTextChanged(const QString&)));
	addConnection(m_pDocument->undoStack(), SIGNAL(redoTextChanged(const QString&)), this, SLOT(redoTextChanged(const QString&)));
}

// Destructor.
SceneModeState::~SceneModeState()
{
}

// Called when the mode is entered.
void SceneModeState::onEntry(QEvent *pEvent)
{
	ModeState::onEntry(pEvent);

	// Initialize undo / redo
	undoRedoChanged();
}

// Called when the mode is exited.
void SceneModeState::onExit(QEvent *pEvent)
{
	ModeState::onExit(pEvent);
}

// Update undo / redo state.
void SceneModeState::undoRedoChanged()
{
	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();
	mainWindow.actionUndo->setEnabled(m_pDocument->undoStack()->canUndo());
	undoTextChanged(m_pDocument->undoStack()->undoText());
	mainWindow.actionRedo->setEnabled(m_pDocument->undoStack()->canRedo());
	redoTextChanged(m_pDocument->undoStack()->redoText());
}

// Update undo text.
void SceneModeState::undoTextChanged(const QString &text)
{
	QString actionText = MainWindow::tr("Undo");

	if (!text.isEmpty())
		actionText += ": " + text;

	m_pEditor->mainWindow()->widgets().actionUndo->setText(actionText);
}

// Update redo text.
void SceneModeState::redoTextChanged(const QString &text)
{
	QString actionText = MainWindow::tr("Redo");

	if (!text.isEmpty())
		actionText += ": " + text;

	m_pEditor->mainWindow()->widgets().actionRedo->setText(actionText);
}
