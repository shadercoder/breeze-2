#include "stdafx.h"
#include "Tiles/UndoHistoryWidget.h"

#include "Documents/AbstractDocument.h"

#include "Utility/Checked.h"

// Constructor.
UndoHistoryWidget::UndoHistoryWidget(QWidget *pParent, Qt::WFlags flags)
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
	void initialize(MainWindow *pMainWindow) const
	{
		QDockWidget *pDock = new QDockWidget(pMainWindow);
		pDock->setObjectName("UndoHistoryWidget");
		pDock->setWidget( new UndoHistoryWidget(pDock) );
		pDock->setWindowTitle(pDock->widget()->windowTitle());
		pDock->setWindowIcon(pDock->widget()->windowIcon());
		
		// Visible by default
		pMainWindow->addDockWidget(Qt::BottomDockWidgetArea, pDock);

		checkedConnect(pMainWindow->widgets().actionUndo_History, SIGNAL(triggered()), pDock, SLOT(show()));
		checkedConnect(pMainWindow, SIGNAL(documentChanged(AbstractDocument*)), pDock->widget(), SLOT(setDocument(AbstractDocument*)));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const UndoHistoryWidgetPlugin UndoHistoryWidgetPlugin;

} // namespace
