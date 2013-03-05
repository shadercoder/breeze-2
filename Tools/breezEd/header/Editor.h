#ifndef EDITOR_H
#define EDITOR_H

#include "breezEd.h"
#include <lean/tags/noncopyable.h>
#include <lean/smart/scoped_ptr.h>

class MainWindow;
class QSettings;
class DocumentManager;
class DeviceManager;

class Editor : public lean::noncopyable
{
private:
	lean::scoped_ptr<QSettings> m_pSettings;
	lean::scoped_ptr<DocumentManager> m_pDocumentManager;
	lean::scoped_ptr<MainWindow> m_pMainWindow;
	lean::scoped_ptr<DeviceManager> m_pDeviceManager;

public:
	/// Constructor.
	Editor();
	/// Destructor.
	~Editor();

	/// Shows the given message for the given amount of time.
	void showMessage(const QString &msg, int timeout = 0);
	/// Sets the current info text.
	void setInfo(const QString &msg);
	/// Writes the given text.
	void write(const QString &text);
	/// Begins a new console line.
	void newLine();

	/// Gets the settings.
	LEAN_INLINE QSettings* settings() { return m_pSettings.get(); };
	/// Gets the settings.
	LEAN_INLINE const QSettings* settings() const { return m_pSettings.get(); };
	
	/// Gets the document manager.
	LEAN_INLINE DocumentManager* documentManager() { return m_pDocumentManager.get(); };
	/// Gets the document manager.
	LEAN_INLINE const DocumentManager* documentManager() const { return m_pDocumentManager.get(); };

	/// Gets the main window.
	LEAN_INLINE MainWindow* mainWindow() { return m_pMainWindow.get(); };
	/// Gets the main window.
	LEAN_INLINE const MainWindow* mainWindow() const { return m_pMainWindow.get(); };

	/// Gets the main window.
	LEAN_INLINE DeviceManager* deviceManager() { return m_pDeviceManager.get(); };
	/// Gets the main window.
	LEAN_INLINE const DeviceManager* deviceManager() const { return m_pDeviceManager.get(); };
};

template <class Parameter>
class PluginManager;

/// Gets the editor plugin manager.
PluginManager<Editor*>& editorPlugins();

#endif
