#ifndef BROWSEWIDGET_H
#define BROWSEWIDGET_H

#include "ui_BrowseWidget.h"

/// Controller builder tool.
class BrowseWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::BrowseWidget ui;

public:
	/// Constructor.
	BrowseWidget(QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~BrowseWidget();

	/// Installs the given event handler on all relevant child widgets.
	void installFocusHandler(QObject *handler);

	/// Gets the path.
	QString path() const;

public Q_SLOTS:
	/// Sets the path.
	void setPath(const QString &path);

Q_SIGNALS:
	/// Browser requested.
	void browse();
	/// Path editing started.
	void editingStarted();
	/// Path editing finished.
	void editingFinished();
};

#endif
