#include "stdafx.h"
#include "Tiles/ConsoleWidget.h"

#include "Windows/MainWindow.h"
#include "Docking/DockContainer.h"

#include <lean/smart/scoped_ptr.h>

#include "Utility/IconDockStyle.h"
#include "Utility/Checked.h"

// Constructor.
ConsoleWidget::ConsoleWidget(Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
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

// Writes the given line to the console.
void ConsoleWidget::writeLine(const QString &msg)
{
	write(msg);
	write("\n");
}

// Adds the console widget to the given main window.
ConsoleWidget* addConsoleWidget(MainWindow &mainWindow, Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
{
/*	lean::scoped_ptr<QDockWidget> dock( new QDockWidget(&mainWindow) );
	dock->setObjectName("ConsoleWidget");
	dock->setStyle( new IconDockStyle(dock, dock->style()) );
*/
	lean::scoped_ptr<ConsoleWidget> widget( new ConsoleWidget(pEditor, pParent, flags) );
	
/*	dock->setWidget(pConsole);
	dock->setWindowTitle(pConsole->windowTitle());
	dock->setWindowIcon(dock->widget()->windowIcon());
*/	
	// Visible by default
//	mainWindow.addDockWidget(Qt::BottomDockWidgetArea, dock);
	if (!pParent)
	{
		lean::scoped_ptr<DockWidget> dock( DockWidget::wrap(widget) );
		mainWindow.dock()->addDock(dock, DockPlacement::Emplace, DockOrientation::Vertical);
		QObject::connect(mainWindow.widgets().actionConsole, &QAction::triggered, dock, &DockWidget::showAndRaise);
		dock.detach();
	}

	return widget.detach();
}
