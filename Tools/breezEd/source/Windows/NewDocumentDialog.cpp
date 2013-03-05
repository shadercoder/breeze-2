#include "stdafx.h"
#include "Windows/NewDocumentDialog.h"

#include "Editor.h"

#include "Documents/AbstractDocument.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QSettings>

namespace
{

/// Initializes the fields of the given dialog.
void initFields(Ui::NewDocumentDialog &dialog, const QSettings &settings)
{
	dialog.nameEdit->setText( AbstractDocument::tr("<Untitled>") );
	dialog.pathEdit->setText( settings.value("newDocumentDialog/defaultPath", QDir::currentPath()).toString() );
}

/// Updates the type list of the given dialog.
void updateTypes(Ui::NewDocumentDialog &dialog, Editor &editor)
{
	dialog.typeList->clear();

	QList<DocumentType> documentTypes = editor.documentManager()->documentTypes();
	QString defaultDocumentType = editor.settings()->value("newDocumentDialog/defaultDocumentType", "").toString();

	// Loop over document types
	Q_FOREACH (const DocumentType &documentType, documentTypes)
	{
		// Add type to list
		QListWidgetItem *pItem = new QListWidgetItem(documentType.icon, documentType.name);
		dialog.typeList->addItem(pItem);
		
		// Restore defaults
		if (documentType.name == defaultDocumentType)
			pItem->setSelected(true);
	}
}

/// Gets the selected document type.
DocumentType selectedType(const Ui::NewDocumentDialog &dialog, Editor &editor)
{
	QList<QListWidgetItem*> selectedItems = dialog.typeList->selectedItems();

	// Only one document type permitted
	if (selectedItems.count() == 1)
		// Look up document type by name
		return editor.documentManager()->documentType( selectedItems.front()->text() );
	else
		// No document type selected
		return DocumentType();
}

} // namespace

// Constructor.
NewDocumentDialog::NewDocumentDialog(Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: QDialog(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	ui.setupUi(this);
	updateTypes(ui, *m_pEditor);
	initFields(ui, *m_pEditor->settings());
	typeSelectionChanged();
}

// Destructor.
NewDocumentDialog::~NewDocumentDialog()
{
}

// Browses for a document path.
void NewDocumentDialog::browseForPath()
{
	// Open folder browser dialog
	QString newPath = QFileDialog::getExistingDirectory(this, tr("Select a path for the new document."),
		ui.pathEdit->text(), QFileDialog::ShowDirsOnly);
	
	// Check and set new path
	if (!newPath.isEmpty())
		ui.pathEdit->setText(newPath);
}

// Updates the current document type.
void NewDocumentDialog::typeSelectionChanged()
{
	DocumentType documentType = selectedType(ui, *m_pEditor);

	// Check if a valid document type has been selected
	if (documentType.valid())
		// Update description
		ui.descLabel->setText(documentType.description);
	else
		ui.descLabel->setText(tr("Select a document type."));
}

// Validates dialog data and accepts if valid.
void NewDocumentDialog::accept()
{
	DocumentType documentType = selectedType(ui, *m_pEditor);

	m_pEditor->settings()->setValue("newDocumentDialog/defaultPath", ui.pathEdit->text());
	m_pEditor->settings()->setValue("newDocumentDialog/defaultDocumentType", documentType.name);

	// Check document type
	if (!documentType.valid())
	{
		QMessageBox::critical( nullptr,
				tr("Invalid document type."),
				tr("Please select a document type.")
			);
		return;
	}

	// Get name
	QString documentName = ui.nameEdit->text();

	// Validate name
	if (!QRegExp("[a-zA-Z0-9_ .]+").exactMatch(documentName))
	{
		QMessageBox::critical( nullptr,
				tr("Invalid name."),
				tr("The specified name '%1' is invalid. Only use 'a'-'z', 'A'-'Z', '0'-'9', '_', '.' or whitespaces in your document names.").arg(documentName)
			);
		return;
	}

	// Get path
	QDir documentDir(ui.pathEdit->text());
	QString documentPath = documentDir.absolutePath();

	// Validate absolute path
	if (documentPath.isEmpty())
	{
		QMessageBox::critical( nullptr,
				tr("Invalid path."),
				tr("The specified path '%1' is invalid.").arg(documentDir.path())
			);
		return;
	}

	// Check if path existent
	if (!documentDir.exists(documentPath))
	{
		QMessageBox::StandardButton result = QMessageBox::question( nullptr,
				tr("Create path?"),
				tr("The specified path '%1' does not exist. Create?").arg(documentPath),
				QMessageBox::Yes | QMessageBox::Cancel
			);

		// Prompt to create path
		if (result != QMessageBox::Yes)
			return;

		// Try to create path
		QDir::current().mkpath(documentPath);

		// Check if path existent
		if (!documentDir.exists())
		{
			QMessageBox::critical( nullptr,
					tr("Error while creating path."),
					tr("The path '%1' could not be created.").arg(documentPath)
				);
			return;
		}
	}

	// Store accepted values
	m_documentType = documentType;
	m_documentName = documentName;
	m_documentPath = documentPath;

	// Close dialog
	QDialog::accept();
}
