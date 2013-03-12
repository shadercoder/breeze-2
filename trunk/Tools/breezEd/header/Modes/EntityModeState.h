#ifndef ENTITYMODESTATE_H
#define ENTITYMODESTATE_H

#include "breezEd.h"
#include "ModeState.h"

// Prototypes
class SceneDocument;
class Editor;

class SelectInteraction;
class TranslateInteraction;
class RotateInteraction;
class ScaleInteraction;

/// Mode class
class EntityModeState : public ModeState
{
	Q_OBJECT

private:
	SceneDocument *m_pDocument;

	Editor *m_pEditor;

	SelectInteraction *m_pSelectInteraction;
	TranslateInteraction *m_pTranslateInteraction;
	RotateInteraction *m_pRotateInteraction;
	ScaleInteraction *m_pScaleInteraction;

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent);

public:
	/// Constructor.
	EntityModeState(SceneDocument* pDocument, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~EntityModeState();

	/// Gets the document.
	SceneDocument* document() const { return m_pDocument; };

public Q_SLOTS:
	/// Enables the select tool.
	void enableSelect(bool bEnable = true);
	/// Enables the translate tool.
	void enableTranslate(bool bEnable = true);
	/// Enables the rotate tool.
	void enableRotate(bool bEnable = true);
	/// Enables the scale tool.
	void enableScale(bool bEnable = true);

	/// Clones the selection.
	void duplicate();
	/// Removes the selection.
	void remove();
};

#endif
