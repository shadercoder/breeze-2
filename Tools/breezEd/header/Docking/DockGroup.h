#ifndef DOCKGROUP_H
#define DOCKGROUP_H

#include <QtWidgets/QWidget>

/// Dock widget.
class DockGroup : public QWidget
{
	Q_OBJECT

private:
	QLabel *m_icon;
	QLabel *m_title;
	QStackedLayout *m_body;
	QWidget *m_tabs;
	bool m_bTrackChildren;
	bool m_bDragging;
	QPoint m_dragStartPos;

	/// The given widget was added.
	void performAdd(QWidget *widget);
	/// The given widget has been removed.
	void performRemove(QWidget *widget);
	/// The active widget changed.
	void widgetChanged(int idx);
	/// The active widget was closed.
	void widgetClosed();

protected:
	void childEvent(QChildEvent *event);
	bool eventFilter(QObject *sender, QEvent *event);

public:
	/// Constructor.
	DockGroup(QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~DockGroup();

	/// Adds the given widget.
	void addWidget(QWidget *widget, int tabPos = -1);
	/// Removes the given widget.
	void removeWidget(QWidget *widget);

	/// Shows/hides the tab bar.
	void setTabsVisible(bool bShow);
	/// Sets the current tab.
	void setCurrentTab(QWidget *widget);
	/// Returns the current tab.
	QWidget* currentTab() const;
	/// Returns the precise tab corresponding to the given widget, nullptr if unknown.
	QWidget* tabFromWidget(QWidget *widget) const;
	/// Gets the number of tabs.
	int tabCount() const;

Q_SIGNALS:
	/// Dock is dragged.
	void dockDragged(QWidget *dock, QPoint screenPos);
	/// Dock is dropped.
	void dockDropped(QWidget *dock, QPoint screenPos);
};

#endif
