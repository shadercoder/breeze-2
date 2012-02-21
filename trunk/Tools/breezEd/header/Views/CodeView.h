#ifndef CODEVIEW_H
#define CODEVIEW_H

#include "breezEd.h"
#include "AbstractView.h"
#include "ui_CodeView.h"

#include <lean/smart/scoped_ptr.h>

class Editor;
class CodeDocument;
class Mode;

class CodeView : public AbstractView
{
	Q_OBJECT

private:
	Ui::CodeView ui;

	Editor *m_pEditor;

	CodeDocument *m_pDocument;

	Mode *m_pViewMode;

public:
	/// Constructor.
	CodeView(CodeDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~CodeView();

	/// Activates this view.
	void activate();

	/// Text edit.
	QPlainTextEdit* textEdit() { return ui.codeEditor; }
	/// Text edit.
	const QPlainTextEdit* textEdit() const { return ui.codeEditor; }
};

#endif