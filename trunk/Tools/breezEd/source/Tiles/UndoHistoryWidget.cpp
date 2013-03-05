#include "stdafx.h"
#include "Tiles/UndoHistoryWidget.h"

#include "Documents/AbstractDocument.h"

#include "Utility/IconDockStyle.h"
#include "Utility/Checked.h"

#include <lean/smart/scoped_ptr.h>

// Constructor.
UndoHistoryWidget::UndoHistoryWidget(QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags)
{
	ui.setupUi(this);

	ui.undoView->setEmptyLabel(tr("Original state"));
	ui.undoView->setCleanIcon( QIcon(":/breezEd/icons/file/save") );
}

// Destructor.
UndoHistoryWidget::~UndoHistoryWidget()
{
}

// Sets the undo stack.
void UndoHistoryWidget::setUndoStack(QUndoStack *pStack)
{
	ui.undoView->setStack(pStack);
}

// Sets the undo stack of the given document.
void UndoHistoryWidget::setDocument(AbstractDocument *pDocument)
{
	setUndoStack( (pDocument) ? pDocument->undoStack() : nullptr );
}

// Gets the undo stack.
QUndoStack* UndoHistoryWidget::undoStack()
{
	return ui.undoView->stack();
}

// Gets the undo stack.
const QUndoStack* UndoHistoryWidget::undoStack() const
{
	return ui.undoView->stack();
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"
#include "Docking/DockContainer.h"

namespace
{

/// Plugin class.
struct UndoHistoryWidgetPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	UndoHistoryWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~UndoHistoryWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *mainWindow) const
	{
/*		lean::scoped_ptr<QDockWidget> dock( new QDockWidget(mainWindow) );
		dock->setObjectName("UndoHistoryWidget");
		dock->setStyle( new IconDockStyle(dock, dock->style()) );

		dock->setWidget( new UndoHistoryWidget(dock) );
		dock->setWindowTitle(dock->widget()->windowTitle());
		dock->setWindowIcon(dock->widget()->windowIcon());
*/
		lean::scoped_ptr<UndoHistoryWidget> undoHistory( new UndoHistoryWidget() );
		lean::scoped_ptr<DockWidget> dock( DockWidget::wrap(undoHistory) );

		// Visible by default
		mainWindow->dock()->addDock(dock, DockPlacement::Emplace, DockOrientation::Vertical);
//		pMainWindow->addDockWidget(Qt::BottomDockWidgetArea, dock);

		QObject::connect(mainWindow->widgets().actionUndo_History, &QAction::triggered, dock, &DockWidget::showAndRaise);
		QObject::connect(mainWindow, &MainWindow::documentChanged, undoHistory, &UndoHistoryWidget::setDocument);

		undoHistory.detach();
		dock.detach();
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const UndoHistoryWidgetPlugin UndoHistoryWidgetPlugin;

} // namespace
