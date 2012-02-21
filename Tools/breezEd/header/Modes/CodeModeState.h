#ifndef CODEMODESTATE_H
#define CODEMODESTATE_H

#include "breezEd.h"
#include "ModeState.h"

// Prototypes
class CodeDocument;
class Editor;

/// Mode class
class CodeModeState : public ModeState
{
	Q_OBJECT

private:
	// Document
	CodeDocument *m_pDocument;

	// Editor
	Editor *m_pEditor;

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent);

public:
	/// Constructor.
	CodeModeState(CodeDocument* pDocument, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~CodeModeState();

	/// Gets the document.
	CodeDocument* document() const { return m_pDocument; };

public Q_SLOTS:
	/// Update undo / redo state.
	void undoRedoChanged();
};

#endif
