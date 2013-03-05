#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class Editor;
class Mode;
class AbstractDocument;
class ConsoleWidget;
class DockContainer;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	/// Document modes for open documents.
	typedef QMap<AbstractDocument*, Mode*> document_mode_map;

private:
	Ui::MainWindow ui;
	QLabel *m_pInfoLabel;

	Editor *m_pEditor;

	Mode *m_pModeStack;
	document_mode_map m_documentModes;

	DockContainer *m_dock;

	ConsoleWidget *m_pConsole;

	AbstractDocument *m_pDocument;

private Q_SLOTS:
	/// Keeps track of document selections.
	void documentActivated(QMdiSubWindow *pWindow);
	/// Keeps track of document life time.
	void documentClosing(AbstractDocument *pDocument);

protected:
	/// Intercepts the close event.
	void closeEvent(QCloseEvent *pEvent);

public:
	/// Constructor.
	MainWindow(Editor *pEditor, QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~MainWindow();

	/// Gets the active document.
	LEAN_INLINE AbstractDocument* activeDocument() const { return m_pDocument; }

	/// Gets the dock container.
	LEAN_INLINE DockContainer* dock() { return m_dock; }

	/// Console.
	LEAN_INLINE ConsoleWidget* console() { return m_pConsole; }
	/// Status bar.
	LEAN_INLINE QStatusBar* statusBar() { return ui.statusBar; }
	/// Status bar.
	LEAN_INLINE QLabel* infoLabel() { return m_pInfoLabel; }

	/// Main window widgets.
	LEAN_INLINE const Ui::MainWindow& widgets() { return ui; }

	/// Main window widgets.
	LEAN_INLINE Editor* editor() const { return m_pEditor; }

public Q_SLOTS:
	/// Opens the new document dialog.
	void newDocument();
	/// Opens a document.
	bool openDocument();
	/// Opens a document of the given type.
	bool openDocument(const QString &documentType);

	/// Adds the given dock widget.
	void addDockWidgetTabified(Qt::DockWidgetArea area, QDockWidget *dockwidget);
	/// Retrieves the dock widget parent to the given widget, potentially overlapping the given dock widget.
	QDockWidget* overlappingDockWidget(QDockWidget *dockwidget, QWidget *potentiallyOverlapping) const;

	/// Adds the given document.
	bool addDocument(AbstractDocument *pDocument, QWidget *pView);

	/// Sets editing information.
	void setEditingInfo(const QString &info = QString());

	// Shows the windows dialog.
	void showWindowsDialog();
	/// Tiles all subwindows.
	void tileWindows();

Q_SIGNALS:
	/// Focus changed.
	void focusChanged(QWidget *old, QWidget *now);
	/// Emitted when the current document has been changed.
	void documentChanged(AbstractDocument *pDocument);
	/// Emitted when the given document has been closed.
	void documentClosed(AbstractDocument *pDocument);
};

template <class Parameter>
class PluginManager;

/// Gets the main window plugin manager.
PluginManager<MainWindow*>& mainWindowPlugins();

#endif
