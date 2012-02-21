#ifndef INPUTPROVIDER_H
#define INPUTPROVIDER_H

#include "breezEd.h"
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtGui/QWidget>

/// State
class InputState
{
friend class InputProvider;

private:
	bool m_bState;
	bool m_bChanged;
	bool m_bHandled;

public:
	/// Constructor.
	explicit InputState(bool bState = false, bool bChanged = false, bool bHandled = false)
		: m_bState(bState),
		m_bChanged(bChanged),
		m_bHandled(bHandled) { }

	/// Silently updates the state.
	void updateSilently(bool bNewState, bool bSilent = true)
	{
		update(bNewState);
		m_bChanged &= !bSilent;
	}
	/// Updates the state.
	void update(bool bNewState)
	{
		m_bChanged = (m_bState != bNewState);
		m_bState = bNewState;
		m_bHandled = !m_bChanged & m_bHandled;
	}
	/// Resets the state.
	void reset()
	{
		m_bChanged = false;
		m_bHandled = false;
	}
	/// Resets the state.
	void reset(bool bState, bool bChanged = false, bool bHandled = false)
	{
		m_bState = bState;
		m_bChanged = bChanged;
		m_bHandled = bHandled;
	}

	/// Get raw value.
	bool state() const { return m_bState; }

	/// Gets if the state has changed.
	bool hasChanged() const { return m_bChanged; }

	/// Marks state as handled.
	void setHandeled(bool bHandled = true)
	{
		m_bHandled = bHandled;
	}
	/// Gets whether this state has already been handled.
	bool wasHandled() const { return m_bHandled; }

	/// True, if state true and not handled yet.
	bool isTrue(bool bOnce = false) const
	{
		return m_bState && !m_bHandled && (!bOnce || m_bChanged);
	}
	/// True, if state false and not handled yet.
	bool isFalse(bool bOnce = false) const
	{
		return !m_bState && !m_bHandled && (!bOnce || m_bChanged);
	}
};

/// Input provider class
class InputProvider : public QObject
{
	Q_OBJECT

public:
	typedef QMap<int, InputState> state_map;

private:
	QWidget *m_pReferenceWidget;

	state_map m_keys;
	state_map m_buttons;

	QPoint m_position;
	QPoint m_delta;
	QPointF m_relativePosition;
	QPointF m_relativeDelta;
	bool m_bNailMouse;
	bool m_bCursorHidden;
	bool m_bAsynchMouse;

	int m_wheelRotation;
	int m_wheelDelta;

public:
	/// Constructor.
	InputProvider(QWidget *pReferenceWidget, QObject *pParent);
	/// Destructor.
	~InputProvider();

	/// Filters input events for the given object.
	virtual bool eventFilter(QObject *pWatched, QEvent *pEvent);

	/// Tracks the mouse outside the event queue.
	virtual void updateMouseAsync();
	/// Tracks input during drag and drop operations.
	virtual void updateWhileDragging(const QWidget *pReciever, const QDragMoveEvent *pDragEvent);

	/// Nails the mouse.
	virtual void nailMouse();

	/// Marks everything as handled & unchanged.
	virtual void step();
	/// Sets everything to released.
	virtual void release();

	/// Checks whether the given key is currently pressed.
	bool keyPressed(Qt::Key key, bool bOnce = false) const
	{
		state_map::const_iterator itKey = m_keys.find(key);
		return (itKey != m_keys.end()) ? itKey->isTrue(bOnce) : false;
	}
	/// Checks whether the given key is currently released.
	bool keyReleased(Qt::Key key, bool bOnce = false) const
	{
		state_map::const_iterator itKey = m_keys.find(key);
		return (itKey != m_keys.end()) ? itKey->isFalse(bOnce) : !bOnce;
	}
	/// Checks whether the given key state has changed.
	bool keyChanged(Qt::Key key) const
	{
		state_map::const_iterator itKey = m_keys.find(key);
		return (itKey != m_keys.end()) ? itKey->hasChanged() : false;
	}
	/// Marks the given key as handled.
	void setKeyHandled(Qt::Key key)
	{
		state_map::iterator itKey = m_keys.find(key);
		
		if (itKey != m_keys.end())
			itKey->setHandeled();
	}

	/// Checks whether the given button is currently pressed.
	bool buttonPressed(Qt::MouseButton button, bool bOnce = false) const
	{
		state_map::const_iterator itButton = m_buttons.find(button);
		return itButton != m_buttons.end() ? itButton->isTrue(bOnce) : false;
	}
	/// Checks whether the given button is currently released.
	bool buttonReleased(Qt::MouseButton button, bool bOnce = false) const
	{
		state_map::const_iterator itButton = m_buttons.find(button);
		return itButton != m_buttons.end() ? itButton->isFalse(bOnce) : !bOnce;
	}
	/// Checks whether the given button state has changed.
	bool buttonChanged(Qt::MouseButton button) const
	{
		state_map::const_iterator itButton = m_buttons.find(button);
		return itButton != m_buttons.end() ? itButton->hasChanged() : false;
	}
	/// Marks the given button as handled.
	void setButtonHandled(Qt::MouseButton button)
	{
		state_map::iterator itButton = m_buttons.find(button);
		
		if (itButton != m_buttons.end())
			itButton->setHandeled();
	}

	/// Gets the current position.
	QPoint position(void) const { return m_position; }
	/// Gets the current relative position.
	QPointF relativePosition(void) const { return m_relativePosition; }
	/// Gets the current delta.
	QPoint delta(void) const { return m_delta; }
	/// Gets the current relative delta.
	QPointF relativeDelta(void) const { return m_relativeDelta; }

	/// Gets the current wheel rotation.
	int wheelRotation(void) const { return m_wheelRotation; }
	/// Gets the current wheel delta.
	int wheelDelta(void) const { return m_wheelDelta; }

	/// Gets the reference widget.
	QWidget* referenceWidget(void) const { return m_pReferenceWidget; }

	/// Checks whether the given key is currently pressed.
	bool operator [](Qt::Key key) const { return keyPressed(key); };
	/// Checks whether the given button is currently pressed.
	bool operator [](Qt::MouseButton button) const { return buttonPressed(button); };
};

#endif