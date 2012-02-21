#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include <QtGui/QWidget>

class AbstractView : public QWidget
{
	Q_OBJECT

public:
	/// Constructor.
	AbstractView(QWidget *pParent = nullptr, Qt::WFlags flags = 0)
		: QWidget(pParent, flags) { }

public Q_SLOTS:
	/// Activates this view.
	virtual void activate() = 0;
};

#endif