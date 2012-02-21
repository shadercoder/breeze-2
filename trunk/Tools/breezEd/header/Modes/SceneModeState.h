#ifndef SCENEMODESTATE_H
#define SCENEMODESTATE_H

#include "breezEd.h"
#include "ModeState.h"

// Prototypes
class SceneDocument;
class Editor;

/// Mode class
class SceneModeState : public ModeState
{
	Q_OBJECT

private:
	// Document
	SceneDocument *m_pDocument;

	// Editor
	Editor *m_pEditor;

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent);

public:
	/// Constructor.
	SceneModeState(SceneDocument* pDocument, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~SceneModeState();

	/// Gets the document.
	SceneDocument* document() const { return m_pDocument; };

public Q_SLOTS:
	/// Update undo / redo state.
	void undoRedoChanged();
	/// Update undo text.
	void undoTextChanged(const QString &text);
	/// Update redo text.
	void redoTextChanged(const QString &text);
};

#endif
