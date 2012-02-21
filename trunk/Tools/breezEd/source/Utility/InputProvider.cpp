#include "stdafx.h"
#include "Utility/InputProvider.h"

#include <QtGui/QApplication>

namespace
{

/// Gets a reference widget.
QWidget* referenceWidget(QWidget *pReferenceWidget, QObject *pSender)
{
	if (!pReferenceWidget)
	{
		pReferenceWidget = qobject_cast<QWidget*>(pSender);

		if (!pReferenceWidget)
			// Use desktop as reference if no other widget available
			pReferenceWidget = QApplication::desktop();
	}

	return pReferenceWidget;
}

/// Transforms the given point from the given source widget to the given destination widget.
QPoint transform(QPoint p, const QWidget *pSource, const QWidget *pDest)
{
	if (pSource)
		p = pSource->mapToGlobal(p);
	if (pDest)
		p = pDest->mapFromGlobal(p);
	return p;
}

/// Computes a relative position from the given absolute position.
QPointF relativePosition(const QPoint &pos, const QWidget &widget)
{
	return QPointF(
			pos.x() / static_cast<float>(widget.width()),
			pos.y() / static_cast<float>(widget.height())
		);
}

/// Updates mouse positions & deltas.
void updateMouse(QPoint &pos, QPointF &relPos, QPoint &delta, QPointF &relDelta, const QPoint &newPos, const QWidget &widget)
{
	delta += newPos - pos;
	pos = newPos;

	relDelta = relativePosition(delta, widget);
	relPos = relativePosition(pos, widget);
}

/// Updates mouse buttons.
void updateMouse(InputProvider::state_map &state, Qt::MouseButtons buttons, bool bSilent = false)
{
	state[Qt::LeftButton].updateSilently( (buttons & Qt::LeftButton) != 0, bSilent );
	state[Qt::MidButton].updateSilently( (buttons & Qt::MidButton) != 0, bSilent );
	state[Qt::RightButton].updateSilently( (buttons & Qt::RightButton) != 0, bSilent );
}

/// Updates keys from the given key modifiers.
void updateKeys(InputProvider::state_map &state, Qt::KeyboardModifiers modifiers, bool bSilent = false)
{
	state[Qt::Key_Shift].updateSilently( (modifiers & Qt::ShiftModifier) != 0, bSilent );
	state[Qt::Key_Control].updateSilently( (modifiers & Qt::ControlModifier) != 0, bSilent );
	state[Qt::Key_Alt].updateSilently( (modifiers & Qt::AltModifier) != 0, bSilent );
}

} // namespace

// Constructor.
InputProvider::InputProvider(QWidget *pReferenceWidget, QObject *pParent)
	: QObject(pParent),
	m_pReferenceWidget( LEAN_ASSERT_NOT_NULL(pReferenceWidget) ),
	m_wheelRotation(0),
	m_wheelDelta(0),
	m_bNailMouse(false),
	m_bCursorHidden(false),
	m_bAsynchMouse(false)
{
}

// Destructor.
InputProvider::~InputProvider()
{
}

// Filters input events for the given object.
bool InputProvider::eventFilter(QObject *pWatched, QEvent *pEvent)
{
	QEvent::Type eventType = pEvent->type();

	switch(eventType)
	{
	// Keyboard input
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		{
			QKeyEvent *pKeyEvent = static_cast<QKeyEvent*>(pEvent);

			// Ignore repetitions
			if (!pKeyEvent->isAutoRepeat())
				// Keep track of key state
				m_keys[pKeyEvent->key()].update(eventType == QEvent::KeyPress);
		}
		break;

	// Mouse input
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
		{
			QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(pEvent);

			// Keep track of button state
			m_buttons[pMouseEvent->button()].update(eventType == QEvent::MouseButtonPress);
		}
		// NOTE: No break, also update position

	// Mouse position input
	case QEvent::MouseMove:
		{
			// Don't overwrite asynchronous mouse position
			if (!m_bAsynchMouse)
			{
				QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(pEvent);

				// Keep track of mouse position
				updateMouse(
						m_position, m_relativePosition, m_delta, m_relativeDelta,
						m_pReferenceWidget->mapFromGlobal( pMouseEvent->globalPos() ),
						*m_pReferenceWidget
					);
			}
		}
		break;

	// Drag'n'drop input
	case QEvent::DragMove:
		{
			QDragMoveEvent *pDragEvent = static_cast<QDragMoveEvent*>(pEvent);

			// Don't overwrite asynchronous mouse position
			if (!m_bAsynchMouse)
			{
				QWidget *pRefWidget = ::referenceWidget(m_pReferenceWidget, pWatched);

				// Keep track of mouse position
				updateMouse(
						m_position, m_relativePosition, m_delta, m_relativeDelta,
						transform( pDragEvent->pos(), qobject_cast<QWidget*>(pWatched), m_pReferenceWidget ),
						*m_pReferenceWidget
					);
			}

			// Mouse buttons & keys
			updateMouse(m_buttons, pDragEvent->mouseButtons(), true);
			updateKeys(m_keys, pDragEvent->keyboardModifiers(), true);
		}
		break;

	// Mouse wheel input
	case QEvent::Wheel:
		{
			QWheelEvent *pWheelEvent = static_cast<QWheelEvent*>(pEvent); 
			
			// Keep track of mouse wheel
			m_wheelDelta = pWheelEvent->delta();
			m_wheelRotation += m_wheelDelta;
		}
		break;

	// No input
	case QEvent::FocusOut:
		release();
		break;
	}

	// Just listening, keep going
	return QObject::eventFilter(pWatched, pEvent);
}

// Tracks the mouse outside the event queue.
void InputProvider::updateMouseAsync()
{
	// Keep track of mouse position
	updateMouse(
			m_position, m_relativePosition, m_delta, m_relativeDelta,
			m_pReferenceWidget->mapFromGlobal( QCursor::pos() ),
			*m_pReferenceWidget
		);

	// Asynch position is trump
	m_bAsynchMouse = true;
}

// Tracks input during drag and drop operations.
void InputProvider::updateWhileDragging(const QWidget *pReciever, const QDragMoveEvent *pDragEvent)
{
	// Don't overwrite asynchronous mouse position
	if (!m_bAsynchMouse)
		// Keep track of mouse position
		updateMouse(
				m_position, m_relativePosition, m_delta, m_relativeDelta,
				transform( pDragEvent->pos(), pReciever, m_pReferenceWidget ),
				*m_pReferenceWidget
			);

	// Mouse buttons & keys
	updateMouse(m_buttons, pDragEvent->mouseButtons(), true);
	updateKeys(m_keys, pDragEvent->keyboardModifiers(), true);
}

// Nails the mouse.
void InputProvider::nailMouse()
{
	m_bNailMouse = true;
}

// Marks everything as handled & unchanged.
void InputProvider::step()
{
	// Reset wheel delta
	m_wheelDelta = 0;

	// Reset mouse
	if (m_bNailMouse)
	{
		if (!m_bCursorHidden)
		{
			m_pReferenceWidget->setCursor( QCursor(Qt::BlankCursor) );
			m_bCursorHidden = true;
		}

		QCursor::setPos( m_pReferenceWidget->mapToGlobal(m_position - m_delta) );
		m_bNailMouse = false;
	}
	else
	{
		if (m_bCursorHidden)
		{
			m_pReferenceWidget->setCursor( QCursor() );
			m_bCursorHidden = false;
		}

		m_delta = QPoint(0, 0);
		m_relativeDelta = QPointF(0.0f, 0.0f);
	}
	
	m_bAsynchMouse = false;

	// Reset keys & buttons
	for (state_map::iterator itState = m_keys.begin(); itState != m_keys.end(); ++itState)
		itState->reset();
	for (state_map::iterator itState = m_buttons.begin(); itState != m_buttons.end(); ++itState)
		itState->reset();
}

// Sets everything to released.
void InputProvider::release()
{
	// Release everything
	for (state_map::iterator itKey = m_keys.begin(); 
		itKey != m_keys.end(); ++itKey)
		itKey->update(false);
	for (state_map::iterator itButton = m_buttons.begin();
		itButton != m_buttons.end(); ++itButton)
		itButton->update(false);
}
