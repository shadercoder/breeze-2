#ifndef MESHIMPORTDIALOG_H
#define MESHIMPORTDIALOG_H

#include <QtGui/QDialog>
#include "ui_MeshImportDialog.h"

class Editor;

class MeshImportDialog : public QDialog
{
	Q_OBJECT

private:
	Editor *m_pEditor;

	Ui::MeshImportDialog ui;

private Q_SLOTS:
	/// Forwards console output to the editor's console.
	void forwardConsoleOutput();

public:
	/// Constructor.
	MeshImportDialog(Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~MeshImportDialog();

	/// Validates dialog data and accepts if valid.
	void accept();

	/// Gets the input file.
	QString input() const;
	/// Gets the output file.
	QString output() const;

public Q_SLOTS:
	/// Sets the input file.
	void setInput(const QString &input);
	/// Sets the output file.
	void setOutput(const QString &output);

	/// Browses for an input file.
	void browseForInput();
	/// Browses for an output file.
	void browseForOutput();
};

#endif
