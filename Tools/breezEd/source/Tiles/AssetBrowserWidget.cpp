#include "stdafx.h"
#include "Tiles/AssetBrowserWidget.h"

#include "Utility/Checked.h"

// Constructor.
AssetBrowserWidget::AssetBrowserWidget(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	ui.setupUi(this);
}

// Destructor.
AssetBrowserWidget::~AssetBrowserWidget()
{
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"

namespace
{

/// Plugin class.
struct AssetBrowserWidgetPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	AssetBrowserWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~AssetBrowserWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *pMainWindow) const
	{
		QDockWidget *pDock = new QDockWidget(pMainWindow);
		pDock->setObjectName("AssetBrowserWidget");
		pDock->setWidget( new AssetBrowserWidget(pMainWindow->editor(), pDock) );
		pDock->setWindowTitle(pDock->widget()->windowTitle());
		pDock->setWindowIcon(pDock->widget()->windowIcon());
		
		// Invisible by default
		pMainWindow->addDockWidget(Qt::LeftDockWidgetArea, pDock);
		pDock->hide();

		checkedConnect(pMainWindow->widgets().actionAsset_Browser, SIGNAL(triggered()), pDock, SLOT(show()));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const AssetBrowserWidgetPlugin AssetBrowserWidgetPlugin;

} // namespace
