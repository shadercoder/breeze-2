#ifndef NEWDOCUMENTDIALOG_H
#define NEWDOCUMENTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_NewDocumentDialog.h"

#include "Documents/DocumentManager.h"

class Editor;

class NewDocumentDialog : public QDialog
{
	Q_OBJECT

private:
	Editor *m_pEditor;

	Ui::NewDocumentDialog ui;

	DocumentType m_documentType;
	QString m_documentName;
	QString m_documentPath;

public:
	/// Constructor.
	NewDocumentDialog(Editor *pEditor, QWidget *pParent = nullptr , Qt::WindowFlags flags = 0);
	/// Destructor.
	~NewDocumentDialog();

	/// Validates dialog data and accepts if valid.
	void accept();

	/// Gets the accepted document type.
	const DocumentType& documentType(void) const { return m_documentType; };
	/// Gets the accepted document name.
	QString documentName(void) const { return m_documentName; };
	/// Gets the accepted document path.
	QString documentPath(void) const { return m_documentPath; };

public Q_SLOTS:
	/// Browses for a document path.
	void browseForPath();
	/// Updates the current document type.
	void typeSelectionChanged();
};

#endif
