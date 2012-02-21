#include "stdafx.h"
#include "Tiles/ConsoleWidget.h"

#include "Windows/MainWindow.h"

#include "Utility/Checked.h"

// Constructor.
ConsoleWidget::ConsoleWidget(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	ui.setupUi(this);
}

// Destructor.
ConsoleWidget::~ConsoleWidget()
{
}

// Writes the given text to the console.
void ConsoleWidget::write(const QString &msg)
{
	QTextCursor cursor = ui.plainTextEdit->textCursor();
	bool bWasAtEnd = cursor.atEnd();

	ui.plainTextEdit->moveCursor(QTextCursor::End);
	ui.plainTextEdit->insertPlainText(msg);

	if (!bWasAtEnd)
		ui.plainTextEdit->setTextCursor(cursor);
}

// Adds the console widget to the given main window.
ConsoleWidget* addConsoleWidget(MainWindow &mainWindow, Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
{
	QDockWidget *pDock = new QDockWidget(&mainWindow);
	pDock->setObjectName("ConsoleWidget");
	
	ConsoleWidget *pConsole = new ConsoleWidget(pEditor, (pParent) ? pParent : pDock, flags);
	pDock->setWidget(pConsole);
	pDock->setWindowTitle(pConsole->windowTitle());
	pDock->setWindowIcon(pDock->widget()->windowIcon());
	
	// Visible by default
	mainWindow.addDockWidget(Qt::BottomDockWidgetArea, pDock);

	checkedConnect(mainWindow.widgets().actionConsole, SIGNAL(triggered()), pDock, SLOT(show()));

	return pConsole;
}
