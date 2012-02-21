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
	BrowseWidget(QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~BrowseWidget();

	/// Gets the path.
	QString path() const;

public Q_SLOTS:
	/// Sets the path.
	void setPath(const QString &path);

Q_SIGNALS:
	/// Browser requested.
	void browse();
};

#endif
