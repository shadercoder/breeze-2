#include "stdafx.h"
#include "Modes/Mode.h"
#include "Modes/AbstractModeState.h"

#include <lean/smart/auto_restore.h>

// Constructor.
Mode::Mode(QObject *pParent)
	: QObject(pParent),
	m_pParent( qobject_cast<Mode*>(pParent) ),
	m_bChildrenChanged(false),
	m_bStateChanged(false),
	m_pDefaultChild(),
	m_pActiveChild(),
	m_bWasEntered(false),
	m_bInChildTransition(false),
	m_bInTransition(false)
{
}

// Destructor.
Mode::~Mode()
{
	// Exit this mode
	exit();
	LEAN_ASSERT(!wasEntered());
	LEAN_ASSERT(!m_pParent || m_pParent->activeChildMode() != this);

	QList<Mode*> children = childModes();

	// Invalidate pointers to this mode
	Q_FOREACH (Mode *pChild, children)
		pChild->m_pParent = nullptr;
}

// Activates the given child mode.
bool Mode::activateChildMode(Mode *pMode)
{
	LEAN_ASSERT_NOT_NULL(pMode);
	LEAN_ASSERT(pMode->parent() == this);

	// Ignore redundant calls
	if (pMode == m_pActiveChild)
		return false;

	LEAN_ASSERT(!pMode->wasEntered());

	if (m_bInChildTransition)
	{
		qWarning("Mode::enterChildMode: another mode is about to be activated, try again later");
		return false;
	}

	// Avoid unpredictable recursions
	lean::auto_restore<bool>(m_bInChildTransition, true);

	// Exit active child mode
	if (m_pActiveChild)
		m_pActiveChild->exit();

	// Check if active child mode exited
	if (!m_pActiveChild)
	{
		// Enter new mode
		m_pActiveChild = pMode;
		return true;
	}
	else
	{
		qWarning("Mode::enterChildMode: active child mode refuses to exit");
		return false;
	}
}

// Deactivates the given child mode.
bool Mode::deactivateChildMode(Mode *pMode)
{
	LEAN_ASSERT_NOT_NULL(pMode);
	LEAN_ASSERT(pMode->parent() == this);

	// Check child mode state
	if (pMode == m_pActiveChild)
	{
		LEAN_ASSERT(!pMode->wasEntered());

		// Deactivate mode
		m_pActiveChild = nullptr;
		return true;
	}
	else
		return false;
}

// Exits the active child mode.
bool Mode::exitActiveChildMode()
{
	// Really exit active child mode (even default child)
	lean::auto_restore<bool>(m_bInChildTransition, true);

	// Try to exit active child mode
	if (m_pActiveChild)
		m_pActiveChild->exit();

	// Check success
	return !m_pActiveChild;
}

// Enters the default mode, if no other child mode is entered.
bool Mode::maybeEnterDefaultChildMode(void)
{
	// Enter default child mode, if set and not about to be overridden
	if (m_pDefaultChild && !m_bInChildTransition)
		return m_pDefaultChild->enter();
	else
		return false;
}

// Sets the default child mode.
bool Mode::setDefaultChildMode(Mode *pMode)
{
	// Check relationship
	if(pMode && pMode->parent() != this)
	{
		qWarning("Mode::setDefaultChildMode: cannot set default child mode that is no child mode of this mode");
		return false;
	}

	// nullptr allowed
	m_pDefaultChild = pMode;

	// Enter default child, if no child mode active yet
	if(!m_pActiveChild)
		maybeEnterDefaultChildMode();

	return true;
}

// Called when the mode is entered.
void Mode::onEntry(QEvent *pEvent)
{
	LEAN_ASSERT(m_enteredState.empty());

	QList<AbstractModeState*> state = modeState();
	QList<AbstractModeState*>::const_iterator itEntered = state.constBegin();

	// Atomic: Either apply full state or none at all
	try
	{
		// Enter state in order
		for (; itEntered != state.constEnd(); ++itEntered)
			(*itEntered)->onEntry(pEvent);

		// Store active state on success
		m_enteredState = state;
	}
	catch (...)
	{
		// Exit state in reverse order on failure
		while (itEntered != state.constBegin())
		{
			--itEntered;
			(*itEntered)->onExit(pEvent);
		}

		throw;
	}
}

// Called when the mode is exited.
void Mode::onExit(QEvent *pEvent)
{
	// Exit state in reverse order
	for (QList<AbstractModeState*>::const_iterator itEntered = m_enteredState.constEnd();
		itEntered != m_enteredState.constBegin(); )
	{
		--itEntered;
		(*itEntered)->onExit(pEvent);
	}

	m_enteredState.clear();
}

// Performs the mode entry.
void Mode::performEntry(QEvent *pEvent)
{
	// Enter this mode
	onEntry(pEvent);
	m_bWasEntered = true;

	// Emit signal
	Q_EMIT entered();

	// Recursively enter children afterwards
	if (m_pActiveChild)
		m_pActiveChild->performEntry(pEvent);
}

// Performs the mode exit.
void Mode::performExit(QEvent *pEvent)
{
	// Recursively exit children first
	if (m_pActiveChild)
		m_pActiveChild->performExit(pEvent);

	// Exit this mode afterwards
	onExit(pEvent);
	m_bWasEntered = false;

	// Emit signal
	Q_EMIT exited();
}

// Enters this mode.
bool Mode::enter()
{
	if (m_bInTransition)
	{
		qWarning("Mode::enter: mode is in transition, try again later");
		return false;
	}

	// Don't exit / re-enter while in transition
	lean::auto_restore<bool>(m_bInTransition, true);

	// Activate this mode
	bool bActivated = (m_pParent) ? m_pParent->activateChildMode(this) : true;
	bool bParentEntered = (!m_pParent || m_pParent->wasEntered());

	// ORDER: This mode refuses to exit from here on

	// Only actually perform entry if activation successful & parent entered
	if (bActivated && bParentEntered && !m_bWasEntered)
	{
		// Call event handlers & update state
		QEvent event(QEvent::None);
		performEntry(&event);
	}

	// Check success
	return m_bWasEntered;
}

// Leaves this mode.
bool Mode::exit()
{
	if (m_bInTransition)
	{
		qWarning("Mode::exit: mode is in transition, try again later");
		return false;
	}

	// Don't exit / re-enter while in transition
	lean::auto_restore<bool>(m_bInTransition, true);

	if (m_bWasEntered)
	{
		// Call event handlers & update state
		QEvent event(QEvent::None);
		performExit(&event);
	}

	// ORDER: This mode refuses to exit until deactivation

	// Deactivate this mode
	bool bDeactivated = (m_pParent) ? m_pParent->deactivateChildMode(this) : true;

	// Enter default mode, if deactivation successful & default mode not this mode
	if (bDeactivated && m_pParent && m_pParent->defaultChildMode() != this)
		m_pParent->maybeEnterDefaultChildMode();

	// Check success
	return !m_bWasEntered;
}

// Adds a child mode to this mode.
void Mode::addMode(Mode *pMode)
{
	LEAN_ASSERT_NOT_NULL(pMode)->setParent(this);
}

// Removes a child mode from this mode.
bool Mode::removeMode(Mode *pMode)
{
	if (pMode && pMode->parent() == this)
	{
		pMode->setParent(nullptr);
		return true;
	}
	else
		return false;
}

// Adds a state object to this mode.
void Mode::addState(AbstractModeState *pState)
{
	LEAN_ASSERT_NOT_NULL(pState)->setParent(this);
}

// Removes a state object from this mode.
bool Mode::removeState(AbstractModeState *pState)
{
	if (pState && pState->parent() == this)
	{
		pState->setParent(nullptr);
		return true;
	}
	else
		return false;
}

// Intercepts object events.
bool Mode::event(QEvent *pEvent)
{
	// Keep track of parent
	if (pEvent->type() == QEvent::ParentChange)
	{
		// Always exit this mode
		exit();
		
		// Update parent
		m_pParent = qobject_cast<Mode*>(this->parent());
	}

    return QObject::event(pEvent);
}

// Intercepts child events.
void Mode::childEvent(QChildEvent *pEvent)
{
	// IMPORTANT: Passed children CANNOT be assumed to be fully constructed
	// -> No metadata
	// -> No casts
	// -> No subclass operations

	// Keep track of children
	if (pEvent->type() == QEvent::ChildAdded || pEvent->type() == QEvent::ChildRemoved)
	{
		m_bChildrenChanged = true;
		m_bStateChanged = true;

		// Properly remove default child
		if (pEvent->type() == QEvent::ChildRemoved && pEvent->child() == m_pDefaultChild)
			setDefaultChildMode(nullptr);
	}

	QObject::childEvent(pEvent);
}

namespace
{

/// Gets all matching children.
template <class Type>
QList<Type> findImmediateChildren(const QObject *parent)
{
	QList<Type> typedChildren;

	const QList<QObject*> &children = parent->children();

	Q_FOREACH (QObject *child, children)
	{
		Type typedChild = qobject_cast<Type>(child);

		if (typedChild)
			typedChildren.append(typedChild);
	}

	return typedChildren;
}

} // namespace

// Gets all child modes.
QList<Mode*> Mode::childModes() const
{
	if (m_bChildrenChanged)
	{
		m_children = findImmediateChildren<Mode*>(this);
		m_bChildrenChanged = false;
	}

	return m_children;
}

// Gets all mode state objects.
QList<AbstractModeState*> Mode::modeState() const
{
	if (m_bStateChanged)
	{
		m_state = findImmediateChildren<AbstractModeState*>(this);
		m_bStateChanged = false;
	}

	return m_state;
}
