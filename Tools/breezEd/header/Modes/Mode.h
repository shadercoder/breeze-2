#ifndef MODE_H
#define MODE_H

#include "breezEd.h"
#include <QtCore/QObject.h>

class AbstractModeState;

/// Abstract mode class
class Mode : public QObject
{
	Q_OBJECT

private:
	Mode *m_pParent;
	
	mutable QList<Mode*> m_children;
	mutable bool m_bChildrenChanged;

	mutable QList<AbstractModeState*> m_state;
	mutable bool m_bStateChanged;
	
	Mode *m_pDefaultChild;
	Mode *m_pActiveChild;

	QList<AbstractModeState*> m_enteredState;
	bool m_bWasEntered;

	bool m_bInChildTransition;
	bool m_bInTransition;

	/// Activates the given child mode.
	bool activateChildMode(Mode *pMode);
	/// Deactivates the given child mode.
	bool deactivateChildMode(Mode *pMode);
	
	/// Enters the default mode, if no other child mode is entered.
	bool maybeEnterDefaultChildMode();
	/// Exits the active child mode.
	bool exitActiveChildMode();

	/// Performs the mode entry.
	void performEntry(QEvent *pEvent);
	/// Performs the mode exit.
	void performExit(QEvent *pEvent);

	/// Called when the mode is entered.
	void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	void onExit(QEvent *pEvent);

protected:
	/// Intercepts object events.
	virtual bool event(QEvent *pEvent);
	/// Intercepts child events.
	virtual void childEvent(QChildEvent *pEvent);

public:
	/// Constructor.
	Mode(QObject *pParent = nullptr);
	/// Destructor.
	~Mode();

	/// Adds a child mode to this mode.
	void addMode(Mode *pMode);
	/// Removes a child mode from this mode.
	bool removeMode(Mode *pMode);

	/// Adds a state object to this mode.
	void addState(AbstractModeState *pState);
	/// Removes a state object from this mode.
	bool removeState(AbstractModeState *pState);

	/// Gets the parent mode.
	Mode* parentMode() const { return m_pParent; };
	/// Gets all child modes.
	QList<Mode*> childModes() const;

	/// Gets all mode state objects.
	QList<AbstractModeState*> modeState() const;

	/// Sets the default child mode.
	bool setDefaultChildMode(Mode *pMode);
	/// Gets the default child mode.
	Mode* defaultChildMode() const { return m_pDefaultChild; };

	/// Gets the active child mode.
	Mode* activeChildMode() const { return m_pActiveChild; };

	/// Gets whether this mode has been entered.
	inline bool wasEntered(void) const { return m_bWasEntered; };

public Q_SLOTS:
	/// Enters this mode.
	bool enter();
	/// Leaves this mode.
	bool exit();

Q_SIGNALS:
	/// The mode was entered.
	void entered();
	/// The mode was left.
	void exited();
};

#endif
