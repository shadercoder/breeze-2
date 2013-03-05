#ifndef SHAPEIMPORTDIALOG_H
#define SHAPEIMPORTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_ShapeImportDialog.h"

class Editor;

class ShapeImportDialog : public QDialog
{
	Q_OBJECT

private:
	Editor *m_pEditor;

	Ui::ShapeImportDialog ui;

private Q_SLOTS:
	/// Forwards console output to the editor's console.
	void forwardConsoleOutput();

public:
	/// Constructor.
	ShapeImportDialog(Editor *pEditor, QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~ShapeImportDialog();

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

/// Browses for a shape file.
QString browseForShape(const QString &currentPath, Editor &editor, QWidget *pParent);

#endif
