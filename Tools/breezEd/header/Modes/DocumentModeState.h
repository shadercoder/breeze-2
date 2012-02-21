#ifndef DOCUMENTMODESTATE_H
#define DOCUMENTMODESTATE_H

#include "breezEd.h"
#include "ModeState.h"

// Prototypes
class AbstractDocument;
class Editor;

/// Mode class
class DocumentModeState : public ModeState
{
	Q_OBJECT

private:
	// Document
	AbstractDocument *m_pDocument;

	// Editor
	Editor *m_pEditor;

	// View mode
	Mode *m_pViewModes;

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent);

public:
	/// Constructor.
	DocumentModeState(AbstractDocument* pDocument, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~DocumentModeState();

	/// Gets the document.
	AbstractDocument* document() const { return m_pDocument; };

	/// Sets the parallel view mode tree.
	void setViewModes(Mode *pViewModes) { m_pViewModes = pViewModes; }
	/// Gets the parallel view mode tree.
	Mode* viewModes() const { return m_pViewModes; }

public Q_SLOTS:
	/// Updates the window title
	void updateWindowTitle();

	/// Saves the document.
	void save();
	/// Saves the document to a different file.
	void saveAs();
};

#endif
