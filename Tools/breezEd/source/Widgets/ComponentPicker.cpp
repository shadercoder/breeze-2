#include "stdafx.h"
#include "Widgets/ComponentPicker.h"

// Constructor.
ComponentPicker::ComponentPicker(QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags)
{
}

// Destructor.
ComponentPicker::~ComponentPicker()
{
}

// Installs the given event handler on all relevant child widgets.
void ComponentPicker::installFocusHandler(QObject *handler)
{
	m_focusHandlers.push_back( LEAN_ASSERT_NOT_NULL(handler) );
}

// Filters focus events.
bool ComponentPicker::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FocusIn)
	{
		Q_FOREACH(QObject *handler, m_focusHandlers)
			if (handler->eventFilter(obj, event))
				return true;
	}

	return QWidget::eventFilter(obj, event);
}
