#include "stdafx.h"
#include "Utility/IconDockStyle.h"

#include <QtWidgets/QStyleOptionDockWidget>
#include <QtGui/QPainter>

IconDockStyle::IconDockStyle(QWidget *widget, QStyle *style)
	: QProxyStyle(style),
	m_widget(widget)
{
}

IconDockStyle::~IconDockStyle() { }

void IconDockStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* pWidget) const
{
	if ((element == QStyle::CE_DockWidgetTitle) && m_widget) // TODO: || element == QStyle::CE_TabBarTabLabel
	{
		QIcon icon = m_widget->windowIcon();

		if (!icon.isNull())
		{
			int width = baseStyle()->pixelMetric(QStyle::PM_ToolBarIconSize);
			int margin = baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);

			if (element == QStyle::CE_DockWidgetTitle)
			{
				QStyleOptionDockWidget adjustedOption( *qstyleoption_cast<const QStyleOptionDockWidget *>(option) );
				adjustedOption.title = QString(width / adjustedOption.fontMetrics.width(" "), QChar(' ')) + adjustedOption.title;
				baseStyle()->drawControl(element, &adjustedOption, painter, pWidget);
			}
/*			else // if (element == QStyle::CE_TabBarTabLabel)
			{
				// MONITOR: Hack, not thread-safe
				QStyleOption *hackedOption = const_cast<QStyleOption*>(option);
				hackedOption->rect = hackedOption->rect.adjusted(width, 0, 0, 0);
				baseStyle()->drawControl(element, hackedOption, painter, pWidget);
				hackedOption->rect = hackedOption->rect.adjusted(-width, 0, 0, 0);
			}
*/
			QPoint iconPos(margin + option->rect.left(), margin + option->rect.center().y() - width / 2);
			return painter->drawPixmap(iconPos, icon.pixmap(width, width));
		}
	}
	
	baseStyle()->drawControl(element, option, painter, pWidget);
}
