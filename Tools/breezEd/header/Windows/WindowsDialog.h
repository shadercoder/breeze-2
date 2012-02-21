#ifndef WINDOWSDIALOG_H
#define WINDOWSDIALOG_H

#include <QtGui/QDialog>
#include "ui_WindowsDialog.h"

class WindowsDialog : public QDialog
{
	Q_OBJECT

private:
	Ui::WindowsDialog ui;

	QList<QWidget*> m_windows;

public:
	/// Constructor.
	WindowsDialog(const QList<QWidget*> &windows, QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~WindowsDialog();

public Q_SLOTS:
	/// Assigns new windows to this dialog.
	void assignWindows(const QList<QWidget*> &windows);
	/// Reacts to selection changes.
	void selectionChanged();
	/// Activates the selected window.
	void activateSelected();
	/// Saves the selected window.
	void saveSelected();
	/// Closes the selected window.
	void closeSelected();
};

#endif
