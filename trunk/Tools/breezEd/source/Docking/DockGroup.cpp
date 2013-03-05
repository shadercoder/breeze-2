#include "stdafx.h"
#include "Docking/DockGroup.h"

#include "Utility/UI.h"

// Constructor.
DockGroup::DockGroup(QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags),
	m_bTrackChildren(false),
	m_bDragging(false)
{
	this->setObjectName("DockGroup");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	QWidget *titleBar = new QWidget(this);
	titleBar->setObjectName("TitleBar");
	titleBar->setBackgroundRole(QPalette::Midlight);
	titleBar->setAutoFillBackground(true);
	QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
	titleLayout->setContentsMargins(6, 0, 0, 0);
	
	m_icon = new QLabel(titleBar);
	m_icon->setObjectName("Icon");
	titleLayout->addWidget(m_icon);
	m_title = new QLabel(titleBar);
	m_title->setObjectName("Title");
	m_title->setText("<empty>");
	titleLayout->addWidget(m_title);
	titleLayout->addSpacerItem( new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum) );
	QToolButton *closeButton = new QToolButton(titleBar);
	closeButton->setObjectName("Close");
	closeButton->setIcon(this->style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
	closeButton->setAutoRaise(true);
	titleLayout->addWidget(closeButton, 0, Qt::AlignCenter);

	layout->addWidget(titleBar);

	m_body = new QStackedLayout();
	connect(m_body, &QStackedLayout::currentChanged, this, &DockGroup::widgetChanged);
	layout->addLayout(m_body);

	m_tabs = new QWidget(this);
	m_tabs->setObjectName("TabBar");
	QHBoxLayout *tabLayout = new QHBoxLayout(m_tabs);
	tabLayout->setMargin(0);
	tabLayout->setSpacing(1);
	
	QWidget *tabSpacer = new QWidget(m_tabs);
	tabSpacer->setObjectName("Spacer");
	tabSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	tabLayout->addWidget(tabSpacer);
//	tabLayout->addSpacerItem( new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum) );

	layout->addWidget(m_tabs);

	titleBar->installEventFilter(this);
	connect(closeButton, &QAbstractButton::clicked, this, &DockGroup::widgetClosed);

	// Start tracking
	m_bTrackChildren = true;
}

// Destructor.
DockGroup::~DockGroup()
{
	// Stop tracking
	m_bTrackChildren = false;
}

namespace
{

/// Gets an icon for the given widget.
QIcon getIcon(QWidget *widget, DockGroup *dock)
{
	QIcon icon;

	if (widget)
		icon = widget->windowIcon();

	if (icon.isNull())
		icon = (widget ? widget : dock)->style()->standardIcon(QStyle::SP_TitleBarMaxButton);

	return icon;
}

/// Gets a name for the given widget.
QString getName(QWidget *widget)
{
	QString name = DockGroup::tr("<empty>");
	
	if (widget)
	{
		name = widget->windowTitle();

		if (name.isEmpty())
			name = DockGroup::tr("<unnamed>");
	}

	return name;
}

} // namespace

// The active widget changed.
void DockGroup::widgetChanged(int idx)
{
	QWidget *widget = m_body->widget(idx);
	m_icon->setPixmap( getIcon(widget, this).pixmap(m_icon->height()) );
	m_title->setText( getName(widget) );
}

// The active widget was closed.
void DockGroup::widgetClosed()
{
	if (QWidget *widget = m_body->currentWidget())
		widget->close();
}

// Shows/hides the tab bar.
void DockGroup::setTabsVisible(bool bShow)
{
	m_tabs->setVisible(bShow);
}

// Gets the number of tabs.
int DockGroup::tabCount() const
{
	return m_tabs->layout()->count() - 1;
}

namespace
{

// Gets the tab target widget.
QWidget* getTabTarget(QObject *object)
{
	return object->property("tabTargetWidget").value<QWidget*>();
}

QToolButton* getTabButton(const QWidget &tabs, QWidget *widget)
{
	Q_FOREACH(QObject *tab, tabs.children())
		if (widget == getTabTarget(tab))
			return qobject_cast<QToolButton*>(tab);

	return nullptr;
}

} // namespace

// Sets the current tab.
void DockGroup::setCurrentTab(QWidget *widget)
{
	m_body->setCurrentWidget(widget);
}

// Returns the current tab.
QWidget* DockGroup::currentTab() const
{
	return m_body->currentWidget();
}

// Returns the precise tab corresponding to the given widget, nullptr if unknown.
QWidget* DockGroup::tabFromWidget(QWidget *widget) const
{
	return getTabTarget(widget);
}

// The given widget was added.
void DockGroup::performAdd(QWidget *widget)
{
	m_body->addWidget(widget);
	widget->installEventFilter(this);

	QToolButton *tabButton = new QToolButton(m_tabs);
	tabButton->setIcon( getIcon(widget, this) );
	tabButton->setText( getName(widget) );
	tabButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tabButton->setAutoRaise(true);
	tabButton->setMinimumSize(m_tabs->height(), -1);
	tabButton->setToolTip(tabButton->text());
	tabButton->setProperty("tabTargetWidget", QVariant::fromValue(widget));
	connect(tabButton, &QToolButton::clicked, widget, &QWidget::raise);
	tabButton->installEventFilter(this);

	QHBoxLayout *tabLayout = static_cast<QHBoxLayout*>(m_tabs->layout());
	tabLayout->insertWidget(tabCount(), tabButton);
	
/*	struct TabSelect
	{
		QStackedLayout *layout;
		QWidget *target;

		void operator ()() const
		{
			layout->setCurrentWidget(target);
		}
	} tabSelect = { m_body, widget };
	connect(tabButton, &QToolButton::clicked, tabSelect);
*/
	m_tabs->setVisible(tabCount() > 1);
}

// The given widget has been removed.
void DockGroup::performRemove(QWidget *widget)
{
	widget->removeEventFilter(this);
	m_body->removeWidget(widget);
	delete getTabButton(*m_tabs, widget);

	m_tabs->setVisible(tabCount() > 1);
}

// Adds the given widget.
void DockGroup::addWidget(QWidget *widget, int tabPos)
{
	widget->setParent(this);

	if (tabPos >= 0)
	{
		tabPos = min(tabPos, this->tabCount());

		if (QToolButton *tabButton = getTabButton(*m_tabs, widget))
			static_cast<QHBoxLayout*>(m_tabs->layout())->insertWidget(tabPos, tabButton);
	}
}

// Removes the given widget.
void DockGroup::removeWidget(QWidget *widget)
{
	if (widget->parent() == this)
		widget->setParent(nullptr);
}

void DockGroup::childEvent(QChildEvent *event)
{
	QWidget::childEvent(event);

	if (m_bTrackChildren && event->isAccepted())
		if (QWidget *widget = qobject_cast<QWidget*>(event->child()))
		{
			if (event->added())
				performAdd(widget);
			else if (event->removed())
				performRemove(widget);
		}
}

bool DockGroup::eventFilter(QObject *sender, QEvent *event)
{
	QEvent::Type type = event->type();

	switch (type)
	{
	case QEvent::ZOrderChange:
		{
			QWidget *widget = static_cast<QWidget*>(sender);

			if (widget->parent() == this && widget == this->children().back())
				m_body->setCurrentWidget(widget);
		}
		break;

	case QEvent::MouseButtonPress:
		{
			m_dragStartPos = static_cast<QMouseEvent*>(event)->screenPos().toPoint();
		}
		break;

	case QEvent::MouseMove:
	case QEvent::MouseButtonRelease:
		{
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

			if (type == QEvent::MouseMove && mouseEvent->buttons() & Qt::LeftButton ||
				type == QEvent::MouseButtonRelease && ~mouseEvent->buttons() & Qt::LeftButton)
			{
				QPoint dragPos = mouseEvent->screenPos().toPoint();

				bool bNowDragging = m_bDragging || (dragPos - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance();
				bNowDragging &= (type != QEvent::MouseButtonRelease);

				if (bNowDragging || m_bDragging)
				{
					QWidget *draggedTab = getTabTarget(sender);
					if (!draggedTab) draggedTab = m_body->currentWidget();

					if (draggedTab)
					{
						Q_EMIT dockDragged(draggedTab, dragPos);
						if (!bNowDragging)
							Q_EMIT dockDropped(draggedTab, dragPos);

						m_bDragging = bNowDragging;
					}
				}
			}
		}
		break;
	}

	return QWidget::eventFilter(sender, event);
}
