#ifndef CODEVIEWMODESTATE_H
#define CODEVIEWMODESTATE_H

#include "breezEd.h"
#include "ModeState.h"

// Prototypes
class CodeView;
class Editor;

/// Mode class
class CodeViewModeState : public ModeState
{
	Q_OBJECT

private:
	// View
	CodeView *m_pView;

	// Editor
	Editor *m_pEditor;

public:
	/// Constructor.
	CodeViewModeState(CodeView* pView, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~CodeViewModeState();

	/// Gets the view.
	CodeView* view() const { return m_pView; }
};

#endif
