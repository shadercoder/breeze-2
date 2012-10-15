#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include "ui_ConsoleWidget.h"

#include <lean/logging/log.h>
#include <lean/logging/log_target.h>

class Editor;
class MainWindow;

/// Console tool.
class ConsoleWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::ConsoleWidget ui;

	Editor *m_pEditor;

public:
	/// Constructor.
	ConsoleWidget(Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~ConsoleWidget();

	/// Writes the given text to the console.
	void write(const QString &msg);
	/// Writes the given line to the console.
	void writeLine(const QString &msg);
};

/// Log target interface.
class ConsoleWidgetLogBinder : public QObject, public lean::log_target
{
private:
	ConsoleWidget *m_pConsole;
	lean::log *m_pLog;
	
public:
	/// Constructor.
	ConsoleWidgetLogBinder(ConsoleWidget *pConsole, lean::log *pLog, QObject *pParent = nullptr)
		: QObject(pParent),
		m_pConsole( LEAN_ASSERT_NOT_NULL(pConsole) ),
		m_pLog( LEAN_ASSERT_NOT_NULL(pLog) )
	{
		m_pLog->add_target(this);
	}
	/// Destructor.
	~ConsoleWidgetLogBinder()
	{
		m_pLog->remove_target(this);
	}

	/// Prints the given message.
	void print(const lean::char_ntri &message)
	{
		m_pConsole->write(QString::fromUtf8(message.c_str()));
	}
};

/// Adds the console widget to the given main window.
ConsoleWidget* addConsoleWidget(MainWindow &mainWindow, Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);

#endif
