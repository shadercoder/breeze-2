#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QtWidgets/QWidget>

/// Dock widget.
class DockWidget : public QWidget
{
	Q_OBJECT

protected:
	bool event(QEvent *event);
	void childEvent(QChildEvent *event);
	void closeEvent(QCloseEvent *event);

	/// Unsets the widget.
	void performUnset(QWidget *widget);
	/// Sets the widget.
	void performSet(QWidget *widget);

public:
	/// Constructor.
	DockWidget(QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~DockWidget();

	/// Wraps the given widget in a dock widget.
	static DockWidget *wrap(QWidget *widget, QWidget *pParent = nullptr, Qt::WindowFlags flags = 0)
	{
		QScopedPointer<DockWidget> result( new DockWidget(pParent, flags) );
		result->setWidget(widget);
		return result.take();
	}

	/// Sets the widget.
	void setWidget(QWidget *widget);
	/// Sets the widget.
	QWidget *widget() const;

public Q_SLOTS:
	void showAndRaise() { show(); raise(); }

Q_SIGNALS:
	/// Parent changed.
	void parentChanged(DockWidget *dock);
	/// Destroyed.
	void destroyed(DockWidget *dock);
	/// Shown.
	void shown(DockWidget *dock);
	/// Closed.
	void closed(DockWidget *dock);
};

#endif
