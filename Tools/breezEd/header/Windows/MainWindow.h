#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"

class Editor;
class Mode;
class AbstractDocument;
class ConsoleWidget;

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
	MainWindow(Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~MainWindow();

	/// Console.
	LEAN_INLINE ConsoleWidget* console() { return m_pConsole; }
	/// Status bar.
	LEAN_INLINE QStatusBar* statusBar() { return ui.statusBar; }
	/// Status bar.
	LEAN_INLINE QLabel* infoLabel() { return m_pInfoLabel; }

	/// Main window widgets.
	LEAN_INLINE const Ui::MainWindow& widgets() { return ui; }

	/// Main window widgets.
	LEAN_INLINE Editor* editor() { return m_pEditor; }

public Q_SLOTS:
	/// Opens the new document dialog.
	void newDocument();
	/// Opens a document.
	bool openDocument();
	/// Opens a document of the given type.
	bool openDocument(const QString &documentType);

	/// Adds the given document.
	bool addDocument(AbstractDocument *pDocument, QWidget *pView);

	/// Sets editing information.
	void setEditingInfo(const QString &info = QString());

	// Shows the windows dialog.
	void showWindowsDialog();
	/// Tiles all subwindows.
	void tileWindows();

Q_SIGNALS:
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
