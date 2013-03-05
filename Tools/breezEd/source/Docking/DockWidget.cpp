#include "stdafx.h"
#include "Docking/DockWidget.h"

#include "Utility/UI.h"

// Constructor.
DockWidget::DockWidget(QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags)
{
	( new QVBoxLayout(this) )->setMargin(0);
}

// Destructor.
DockWidget::~DockWidget()
{
	Q_EMIT destroyed(this);
}

// Unsets the widget.
void DockWidget::performUnset(QWidget *widget)
{
	QLayoutItem *prev = this->layout()->takeAt(0);
	QWidget *prevWidget = (prev) ? prev->widget() : nullptr;

	if (widget == prevWidget)
	{
		// Reset copied window decoration
		if (this->objectName() == widget->objectName())
			this->setObjectName(QString());
		if (this->windowIcon().data_ptr() == widget->windowIcon().data_ptr())
			this->setWindowIcon(QIcon());
		if (this->windowTitle() == widget->windowTitle())
			this->setWindowTitle(QString());

		// IMPORTANT: Manually remove from layout to update state & prevent recursion
		this->layout()->removeWidget(widget);

		// Delete owned widgets
		if (widget->parent() == this)
			widget->deleteLater();
	}
}

// Sets the widget.
void DockWidget::performSet(QWidget *widget)
{
	QLayoutItem *prev = this->layout()->takeAt(0);
	QWidget *prevWidget = (prev) ? prev->widget() : nullptr;

	// IMPORTANT: Re-assignment would be wrong
	if (widget == prevWidget)
		return;

	if (prevWidget)
		performUnset(prevWidget);

	if (widget)
	{
		this->layout()->addWidget(widget);

		// Copy wrapped widget state
		if (widget->isHidden())
			this->hide();
		widget->show();

		// Copy wrapped widget window decoration
		if (this->objectName().isEmpty())
			this->setObjectName(widget->objectName());
		if (this->windowIcon().isNull())
			this->setWindowIcon(widget->windowIcon());
		if (this->windowTitle().isEmpty())
			this->setWindowTitle(widget->windowTitle());
	}
}

// Sets the widget.
void DockWidget::setWidget(QWidget *widget)
{
	// NOTE: Avoid re-parenting recursion via events
	if (widget)
		widget->setParent(this);
	else
		performSet(nullptr);
}

// Sets the widget.
QWidget* DockWidget::widget() const
{
	QLayoutItem *item = layout()->itemAt(0);
	return (item) ? item->widget() : nullptr;
}

void DockWidget::closeEvent(QCloseEvent *event)
{
	// ORDER: Issue FIRST to land in event queue BEFORE delete
	Q_EMIT closed(this);

	// NOTE: Always accepts
	QWidget::closeEvent(event);
}

void DockWidget::childEvent(QChildEvent *event)
{
	if (QWidget *widget = qobject_cast<QWidget*>(event->child()))
	{
		if (event->added())
			performSet(widget);
		else if (event->removed())
			performUnset(widget);
	}
}

bool DockWidget::event(QEvent *event)
{
	// ORDER: Handle first, emit 'post' signals
	bool result = QWidget::event(event);

	switch (event->type())
	{
	case QEvent::ShowToParent:
		Q_EMIT shown(this);
		break;

	case QEvent::ParentChange:
		Q_EMIT parentChanged(this);
		break;
	}

	return result;
}
