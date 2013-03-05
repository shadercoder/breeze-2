#ifndef ICONDOCKSTYLE_H
#define ICONDOCKSTYLE_H

#include "breezEd.h"
#include <QtWidgets/QProxyStyle>

class IconDockStyle : public QProxyStyle
{
private:
	QWidget *m_widget;

public:
	IconDockStyle(QWidget *widget, QStyle *pStyle);
	~IconDockStyle();

	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* pWidget = 0) const LEAN_OVERRIDE;
};

#endif