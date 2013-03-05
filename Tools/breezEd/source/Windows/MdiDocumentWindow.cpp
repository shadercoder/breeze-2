#include "stdafx.h"
#include "Windows/MdiDocumentWindow.h"

#include "Utility/Checked.h"

namespace
{

/// Prompts the user about saving before closing.
bool promptToSaveAndClose(AbstractDocument &document)
{
	// Nothing to save if unmodified or still open somewhere else
	if(!document.changed() || document.references() > 1)
		return true;

	// Prompt to save
	QMessageBox::StandardButton result = QMessageBox::question( nullptr,
			AbstractDocument::tr("Save changes?"),
			AbstractDocument::tr("The document '%1' has been modified. Save changes before closing?").arg(document.name()),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save
		);
	
	// Save, if confirmed
	if(result == QMessageBox::Save)
	{
		if (!document.save())
		{
			QMessageBox::critical( nullptr,
					AbstractDocument::tr("Error while saving."),
					AbstractDocument::tr("Error while saving document '%1' to '%2'.").arg(document.name()).arg(document.file())
				);

			// WARNING: Don't discard!
			result = QMessageBox::Cancel;
		}
	}

	// Close, if not canceled
	return (result != QMessageBox::Cancel);
}

} // namespace

// Constructor.
MdiDocumentWindow::MdiDocumentWindow(AbstractDocument *pDocument, QWidget *pParent, Qt::WindowFlags flags)
	: QMdiSubWindow(pParent, flags),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) )
{
	updateWindowTitle();

	// Track document changes
	checkedConnect(m_pDocument, SIGNAL(nameChanged(const QString&)), this, SLOT(updateWindowTitle()));
	checkedConnect(m_pDocument, SIGNAL(documentChanged(bool)), this, SLOT(updateWindowTitle()));
}

// Destructor.
MdiDocumentWindow::~MdiDocumentWindow()
{
}

// Updates the window title
void MdiDocumentWindow::updateWindowTitle()
{
	// Get document title
	QString windowTitle = m_pDocument->name();

	// Mark changed documents
	if(m_pDocument->changed())
		windowTitle += "*";

	// Update window title
	setWindowTitle(windowTitle);
}

// Intercepts the close event.
void MdiDocumentWindow::closeEvent(QCloseEvent *pEvent)
{
	// Accept or ignore, depending on user input and document state
	if(promptToSaveAndClose(*m_pDocument))
		// Close window
		QMdiSubWindow::closeEvent(pEvent);
	else
		// User cancelled
		pEvent->ignore();
}
