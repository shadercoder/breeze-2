#include "stdafx.h"
#include "Modes/DocumentModeState.h"

#include "Editor.h"
#include "Windows/MainWindow.h"
#include "Documents/AbstractDocument.h"

#include <QtWidgets/QFileDialog>

#include "Utility/Checked.h"

// Constructor.
DocumentModeState::DocumentModeState(AbstractDocument* pDocument, Editor *pEditor, QObject *pParent)
	: ModeState(pParent),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pViewModes()
{
	const Ui::MainWindow &mainWindow = m_pEditor->mainWindow()->widgets();

	// Main window title
	checkedConnect(m_pDocument, SIGNAL(fileChanged(const QString&)), this, SLOT(updateWindowTitle()));
	checkedConnect(m_pDocument, SIGNAL(documentChanged(bool)), this, SLOT(updateWindowTitle()));

	// Enable save functionality on entry
	assignProperty(mainWindow.actionSave, "enabled", true);
	assignProperty(mainWindow.actionSave_As, "enabled", true);
	addConnection(mainWindow.actionSave, SIGNAL(triggered()), this, SLOT(save()));
	addConnection(mainWindow.actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
}

// Destructor.
DocumentModeState::~DocumentModeState()
{
}

// Called when the mode is entered.
void DocumentModeState::onEntry(QEvent *pEvent)
{
	ModeState::onEntry(pEvent);

	// Update main window title
	updateWindowTitle();
}

// Called when the mode is exited.
void DocumentModeState::onExit(QEvent *pEvent)
{
	// Revert main window title
	m_pEditor->mainWindow()->setEditingInfo();

	ModeState::onExit(pEvent);
}

// Updates the window title
void DocumentModeState::updateWindowTitle()
{
	// Get document title
	QString windowTitle = m_pDocument->file();

	// Mark changed documents
	if(m_pDocument->changed())
		windowTitle += "*";

	// Update window title
	m_pEditor->mainWindow()->setEditingInfo(windowTitle);
}

// Saves the document.
void DocumentModeState::save()
{
	// Save document
	if (!m_pDocument->save())
		QMessageBox::critical( nullptr,
				AbstractDocument::tr("Error while saving"),
				AbstractDocument::tr("Error while saving document '%1' to '%2'.").arg(m_pDocument->name()).arg(m_pDocument->file())
			);
}

// Saves the document to a different file.
void DocumentModeState::saveAs()
{
	// Open file browser dialog
	QString newFile = QFileDialog::getSaveFileName(m_pEditor->mainWindow(), MainWindow::tr("Select a file to save to."), m_pDocument->file());
	
	// Check and set new path
	if (!newFile.isEmpty())
		if (!m_pDocument->saveAs(newFile))
			QMessageBox::critical( nullptr,
					AbstractDocument::tr("Error while saving"),
					AbstractDocument::tr("Error while saving document '%1' at '%2' as '%3'.").arg(m_pDocument->name()).arg(m_pDocument->file()).arg(newFile)
				);
}
