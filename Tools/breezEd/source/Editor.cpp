#include "stdafx.h"
#include "Editor.h"

#include "Windows/MainWindow.h"
#include "Documents/DocumentManager.h"
#include <QtCore/QSettings>
#include "Plugins/PluginManager.h"
#include "Tiles/ConsoleWidget.h"
#include "DeviceManager.h"

#include <lean/logging/errors.h>

// Constructor.
Editor::Editor()
	: m_pSettings( new QSettings("breeze", "breezEd") ),
	m_pDocumentManager( new DocumentManager() )
{
	editorPlugins().initializePlugins(this);

	// ORDER: Create window after plugins have been initialized
	m_pMainWindow = new MainWindow(this);
	m_pMainWindow->setAttribute(Qt::WA_QuitOnClose);

	// ORDER: Create devices after window has been initialized
	try
	{
		m_pDeviceManager = new DeviceManager(m_pMainWindow->winId(), *m_pSettings);
	}
	catch (const std::exception &error)
	{
		LEAN_LOG_ERROR_XMSG("Error while creating devices, you might need to adjust your settings.", error.what());
	}
	catch (...)
	{
		LEAN_LOG_ERROR_MSG("Error while creating devices, you might need to adjust your settings.");
	}
}

// Destructor.
Editor::~Editor()
{
	editorPlugins().finalizePlugins(this);
}

// Shows the given message for the given amount of time.
void Editor::showMessage(const QString &msg, int timeout)
{
	m_pMainWindow.get()->statusBar()->showMessage(msg, timeout);
}

// Sets the current info text.
void Editor::setInfo(const QString &msg)
{
	m_pMainWindow.get()->infoLabel()->setText(msg);
}

// Writs the given text.
void Editor::write(const QString &text)
{
	m_pMainWindow.get()->console()->writeLine(text);
}

// Begins a new console line.
void Editor::newLine()
{
	write(QString());
}

// Gets the editor plugin manager.
PluginManager<Editor*>& editorPlugins()
{
	static PluginManager<Editor*> pluginManager;
	return pluginManager;
}
