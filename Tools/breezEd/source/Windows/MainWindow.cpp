#include "stdafx.h"
#include "Windows/MainWindow.h"

#include "Editor.h"
#include "Plugins/PluginManager.h"

#include "Windows/NewDocumentDialog.h"
#include "Documents/DocumentManager.h"
#include "Documents/AbstractDocument.h"
#include "Documents/AbstractDocumentFactory.h"

#include "Modes/Mode.h"
#include "Windows/MdiDocumentWindow.h"

#include "Views/AbstractView.h"

#include <QtGui/QFileDialog>

#include "Windows/WindowsDialog.h"
#include "Tiles/ConsoleWidget.h"

#include "Utility/Checked.h"

#include <lean/logging/errors.h>

namespace
{

/// Restores the docking layout.
void restoreLayout(QMainWindow &window, const Editor &editor)
{
	window.restoreState(editor.settings()->value("mainWindow/windowState").toByteArray());
}

/// Initializes window state and docking layout.
void restoreWindow(QMainWindow &window, const Editor &editor)
{
	window.restoreGeometry(editor.settings()->value("mainWindow/geometry").toByteArray());
	restoreLayout(window, editor);
}

/// Saves docking layout.
void saveLayout(const QMainWindow &window, Editor &editor)
{
	editor.settings()->setValue("mainWindow/windowState", window.saveState());
}

/// Saves window state and docking layout.
void saveWindow(const QMainWindow &window, Editor &editor)
{
	saveLayout(window, editor);
	editor.settings()->setValue("mainWindow/geometry", window.saveGeometry());
}

/// Builds the open as menu.
void addDocumentTypes(MainWindow &mainWindow, Editor &editor)
{
	QList<DocumentType> documentTypes = editor.documentManager()->documentTypes();

	// Create 'Open As'-menu
	QMenu *pOpenMenu = new QMenu(&mainWindow);
	mainWindow.widgets().actionOpen_As->setMenu(pOpenMenu);

	QSignalMapper *pSignalMapper = new QSignalMapper(pOpenMenu);

	Q_FOREACH (const DocumentType &documentType, documentTypes)
	{
		// Add one open action for each document type
		QAction *pAction = pOpenMenu->addAction(documentType.icon, documentType.name);

		// Map action to document type
		pSignalMapper->setMapping(pAction, documentType.name);
		checkedConnect(pAction, SIGNAL(triggered()), pSignalMapper, SLOT(map()));
	}

	checkedConnect(pSignalMapper, SIGNAL(mapped(const QString&)), &mainWindow, SLOT(openDocument(const QString&)));

	// Add 'Open As'-menu to toolbar
	QToolButton *pOpenButton = qobject_cast<QToolButton*>(
		mainWindow.widgets().fileToolBar->widgetForAction(mainWindow.widgets().actionOpen) );
	pOpenButton->setMenu(pOpenMenu);
	pOpenButton->setPopupMode(QToolButton::MenuButtonPopup);
}

/// Adds a document window.
bool addDocumentWindow(const Ui::MainWindow &mainWindow, QWidget *pView, AbstractDocument *pDocument, Mode *pDocumentMode, const DocumentType &documentType, Editor *pEditor)
{
	// Create document window
	std::auto_ptr<MdiDocumentWindow> pWindow( new MdiDocumentWindow(pDocument) );
	pWindow->setAttribute(Qt::WA_DeleteOnClose);

	// Create default view, if none given
	if (!pView)
	{
		try
		{
			pView = documentType.pFactory->createDocumentView(pDocument, pDocumentMode, pEditor);
		}
		catch (const std::exception &error)
		{
			QMessageBox msg;
			msg.setIcon(QMessageBox::Critical);
			msg.setWindowTitle( MainWindow::tr("Error creating document view") );
			msg.setText( MainWindow::tr("Error while creating document view, you might need to adjust your settings.") );
			msg.setInformativeText( QString::fromUtf8(error.what()) );
			msg.exec();
			return false;
		}
		catch (...)
		{
			QMessageBox::critical( nullptr,
					MainWindow::tr("Error creating document view"),
					MainWindow::tr("Error while creating document view, you might need to adjust your settings.")
				);
			return false;
		}
	}

	pWindow->setWidget(pView);

	// ORDER: Add subwindow after FULL CONSTRUCTION (activation signals ...)
	mainWindow.mdiArea->addSubWindow(pWindow.get());
	pWindow.release()->show();

	return true;
}

/// Adds a document.
Mode* maybeAddDocumentMode(MainWindow::document_mode_map &modes, Mode &modeStack,
	AbstractDocument *pDocument, const DocumentType &documentType,
	MainWindow &mainWindow, Editor *pEditor)
{
	MainWindow::document_mode_map::const_iterator itMode = modes.constFind(pDocument);

	if (itMode == modes.constEnd())
	{
		std::auto_ptr<Mode> pMode( documentType.pFactory->createDocumentMode(pDocument, pEditor, &modeStack) );

		// Keep track of document life time
		checkedConnect(pDocument, SIGNAL(documentClosing(AbstractDocument*, bool)),
			&mainWindow, SLOT(documentClosing(AbstractDocument*)));

		modes[pDocument] = pMode.get();

		return pMode.release();
	}
	else
		return *itMode;
}

} // namespace

// Constructor.
MainWindow::MainWindow(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QMainWindow(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pModeStack( new Mode(this) )
{
	ui.setupUi(this);
	m_pInfoLabel = new QLabel(ui.statusBar);
	m_pInfoLabel->setMinimumWidth(18);
	ui.statusBar->addPermanentWidget(m_pInfoLabel);

	m_pConsole = addConsoleWidget(*this, m_pEditor);
	new ConsoleWidgetLogBinder(m_pConsole, &lean::error_log(), m_pConsole);
	new ConsoleWidgetLogBinder(m_pConsole, &lean::info_log(), m_pConsole);

	checkedConnect(ui.actionNew, SIGNAL(triggered()), this, SLOT(newDocument()));
	checkedConnect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(openDocument()));
	addDocumentTypes(*this, *m_pEditor);

	mainWindowPlugins().initializePlugins(this);

	restoreWindow(*this, *m_pEditor);

	checkedConnect(ui.mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(documentActivated(QMdiSubWindow*)));

	LEAN_LOG("Starting up");

	m_pModeStack->enter();
}

// Destructor.
MainWindow::~MainWindow()
{
	m_pModeStack->exit();

	LEAN_LOG("Shutting down");

	mainWindowPlugins().finalizePlugins(this);
}

// Sets editing information.
void MainWindow::setEditingInfo(const QString &info)
{
	QString windowTitle = "breezEd";

	if (!info.isEmpty())
		windowTitle += " [" + info + "]";

	setWindowTitle(windowTitle);
}

// Opens the new document dialog.
void MainWindow::newDocument()
{
	NewDocumentDialog newDocumentDialog(m_pEditor, this);

	// Open dialog
	while (NewDocumentDialog::Accepted == newDocumentDialog.exec())
	{
		const DocumentType &documentType = newDocumentDialog.documentType();

		// Check document type
		if (documentType.valid())
		{
			QString documentFile = documentType.pFactory->fileFromDir(newDocumentDialog.documentName(), newDocumentDialog.documentPath());
			bool bCreate = true;

			if (QFile::exists(documentFile))
			{
				QMessageBox::StandardButton result = QMessageBox::question( nullptr,
						tr("Overwrite file?"),
						tr("The document file '%1' already exists. Overwrite?").arg(documentFile),
						QMessageBox::Yes | QMessageBox::Cancel
					);

				// Prompt to overwrite
				if (result != QMessageBox::Yes)
					bCreate = false;
			}

			if (bCreate)
			{
				try
				{
					// Create document
					DocumentReference<AbstractDocument> pDocument = documentType.pFactory->createDocument(documentFile, m_pEditor, this);
					pDocument->setName(newDocumentDialog.documentName());
					
					addDocument(pDocument, nullptr);
				}
				catch (const std::exception &error)
				{
					QMessageBox msg;
					msg.setIcon(QMessageBox::Critical);
					msg.setWindowTitle( MainWindow::tr("Error creating document") );
					msg.setText( MainWindow::tr("An unexpected error occurred while creating '%1' document '%2'.").arg(documentType.name).arg(documentFile) );
					msg.setInformativeText( QString::fromUtf8(error.what()) );
					msg.exec();
				}
				catch (...)
				{
					QMessageBox::critical( nullptr,
							MainWindow::tr("Error creating document"),
							MainWindow::tr("An unexpected error occurred while creating '%1' document '%2'.").arg(documentType.name).arg(documentFile)
						);
				}

				return;
			}
		}
		else
			QMessageBox::critical( nullptr,
					tr("Invalid document type"),
					tr("Unknown document type '%1'.").arg(documentType.name)
				);
	}
}

// Opens a document.
bool MainWindow::openDocument()
{
	// Open file browser dialog
	QString openFile = QFileDialog::getOpenFileName(this, tr("Select a file to open."), QDir::currentPath());

	if (!openFile.isEmpty())
	{
		QList<DocumentType> documentTypes = m_pEditor->documentManager()->documentTypes();
		QList<DocumentType> matchingTypes, anyTypes;

		Q_FOREACH (const DocumentType &documentType, documentTypes)
			// Find matching document types
			switch (documentType.pFactory->matchFile(openFile))
			{
			case FileMatch::Match:
				matchingTypes.push_back(documentType);
				break;
			case FileMatch::Any:
				anyTypes.push_back(documentType);
				break;
			}

		// Check if unambiguous
		if (matchingTypes.size() != 1)
			matchingTypes.append(anyTypes);

		if (!matchingTypes.empty())
		{
			const DocumentType *selectedType = &matchingTypes.first();

			// Resolve ambiguities
			if (matchingTypes.size() > 1)
			{
				QStringList matchingTypeNames;

				// Build list of matching document type names
				Q_FOREACH (const DocumentType &documentType, matchingTypes)
					matchingTypeNames.push_back(documentType.name);

				// Select a matching document type
				bool ok = false;
				QString selectedTypeName = QInputDialog::getItem(this,
					tr("Select a document type."),
					tr("Select a document type for '%1':").arg(openFile),
					matchingTypeNames, 0,
					false, &ok);

				selectedType = (ok)
					? &matchingTypes.at(matchingTypeNames.indexOf(selectedTypeName))
					: nullptr;
			}

			// Check if cancelled during ambiguity resolution
			if (selectedType)
			{
				try
				{
					// Open document
					DocumentReference<AbstractDocument> pDocument = selectedType->pFactory->openDocument(openFile, m_pEditor, this);
				
					if (pDocument)
						return addDocument(pDocument, nullptr);
					else
						QMessageBox::critical( nullptr,
							tr("Error while opening"),
							tr("Could not open '%1' file '%2'.").arg(selectedType->name).arg(openFile)
						);
				}
				catch (const std::exception &error)
				{
					QMessageBox msg;
					msg.setIcon(QMessageBox::Critical);
					msg.setWindowTitle( MainWindow::tr("Error opening document") );
					msg.setText( MainWindow::tr("An unexpected error occurred while opening '%1' document '%2'.").arg(selectedType->name).arg(openFile) );
					msg.setInformativeText( QString::fromUtf8(error.what()) );
					msg.exec();
				}
				catch (...)
				{
					QMessageBox::critical( nullptr,
							MainWindow::tr("Error opening document"),
							MainWindow::tr("An unexpected error occurred while opening '%1' document '%2'.").arg(selectedType->name).arg(openFile)
						);
				}
			}
		}
		else
			QMessageBox::critical( nullptr,
					tr("Unknown document type"),
					tr("No matching document type found for '%1'.").arg(openFile)
				);
	}

	return false;
}

// Opens a document of the given type.
bool MainWindow::openDocument(const QString &documentTypeName)
{
	// Open file browser dialog
	QString openFile = QFileDialog::getOpenFileName(m_pEditor->mainWindow(), tr("Select a '%1' file to open.").arg(documentTypeName), QDir::currentPath());

	if (!openFile.isEmpty())
	{
		const DocumentType &documentType = m_pEditor->documentManager()->documentType(documentTypeName);

		if (documentType.valid())
		{
			try
			{
				// Open document
				DocumentReference<AbstractDocument> pDocument = documentType.pFactory->openDocument(openFile, m_pEditor, this);

				if (pDocument)
					return addDocument(pDocument, nullptr);
				else
					QMessageBox::critical( nullptr,
						tr("Error opening document"),
						tr("Could not open '%1' file '%2'.").arg(documentTypeName).arg(openFile)
					);
			}
			catch (const std::exception &error)
			{
				QMessageBox msg;
				msg.setIcon(QMessageBox::Critical);
				msg.setWindowTitle( MainWindow::tr("Error opening document") );
				msg.setText( MainWindow::tr("An unexpected error occurred while opening '%1' document '%2'.").arg(documentTypeName).arg(openFile) );
				msg.setInformativeText( QString::fromUtf8(error.what()) );
				msg.exec();
			}
			catch (...)
			{
				QMessageBox::critical( nullptr,
						MainWindow::tr("Error opening document"),
						MainWindow::tr("An unexpected error occurred while opening '%1' document '%2'.").arg(documentTypeName).arg(openFile)
					);
			}
		}
		else
			QMessageBox::critical( nullptr,
					tr("Invalid document type"),
					tr("Unknown document type '%1'.").arg(documentTypeName)
				);
	}

	return false;
}

// Adds the given document.
bool MainWindow::addDocument(AbstractDocument *pDocument, QWidget *pView)
{
	LEAN_ASSERT_NOT_NULL(pDocument);

	const DocumentType &documentType = m_pEditor->documentManager()->documentType(pDocument->type());

	if (documentType.valid())
	{
		// Create document mode
		Mode *pMode = maybeAddDocumentMode(m_documentModes, *m_pModeStack, pDocument, documentType, *this, m_pEditor);

		// Create document window
		return addDocumentWindow(ui, pView, pDocument, pMode, documentType, m_pEditor);
	}
	else
		return false;
}

// Intercepts the close event.
void MainWindow::closeEvent(QCloseEvent *pEvent)
{
	QList<QMdiSubWindow*> documentWindows = ui.mdiArea->subWindowList();

	Q_FOREACH (QMdiSubWindow *pDocumentWindow, documentWindows)
	{
		// Try to close document window
		if (!pDocumentWindow->close())
		{
			// Document window refuses to close, cancel
			pEvent->ignore();
			return;
		}
	}

	// Save layout changes
	saveWindow(*this, *m_pEditor);

	// Close main window
	QMainWindow::closeEvent(pEvent);
}

// Keeps track of document selections.
void MainWindow::documentActivated(QMdiSubWindow *pWindow)
{
	MdiDocumentWindow *pDocumentWindow = qobject_cast<MdiDocumentWindow*>(pWindow);

	if (pDocumentWindow)
	{
		AbstractView *pView = qobject_cast<AbstractView*>(pWindow->widget());

		// Activate view
		if (pView)
			pView->activate();

		document_mode_map::const_iterator itDocumentMode = m_documentModes.constFind(pDocumentWindow->document());

		// Enter document mode
		if (itDocumentMode != m_documentModes.constEnd())
			(*itDocumentMode)->enter();

		Q_EMIT documentChanged(pDocumentWindow->document());
	}
}

// Keeps track of document life time.
void MainWindow::documentClosing(AbstractDocument *pDocument)
{
	Q_EMIT documentClosed(pDocument);

	document_mode_map::iterator itDocumentMode = m_documentModes.find(pDocument);

	if (itDocumentMode != m_documentModes.end())
	{
		Mode *pDocumentMode = *itDocumentMode;
		m_documentModes.erase(itDocumentMode);
		pDocumentMode->deleteLater();
	}
}

namespace
{

/// Casts QList<Src> to QList<Dest>.
template <class Dest, class Src>
QList<Dest> list_cast(const QList<Src> &list)
{
	QList<Dest> dest;

	Q_FOREACH (const Src &src, list)
		dest.append( static_cast<Dest>(src) );

	return dest;
}

} // namespace

// Shows the windows dialog.
void MainWindow::showWindowsDialog()
{
	WindowsDialog windowsDialog( list_cast<QWidget*>(ui.mdiArea->subWindowList()), this );
	windowsDialog.exec();
}

// Tiles all subwindows.
void MainWindow::tileWindows()
{
	struct SubWindowOrder
	{
		bool operator ()(QWidget *pLeft, QWidget *pRight)
		{
			QPoint pos1 = pLeft->pos(), pos2 = pRight->pos();
			QSize size1 = pLeft->size(), size2 = pRight->size();

			// Vertical sorting
			if( pos1.y() + size1.height() / 2 > pos2.y() + size2.height() )
				return true;
			else if( pos2.y() + size2.height() / 2 > pos1.y() + size1.height() )
				return false;

			// Horizontal sorting
			return pos1.x() > pos2.x();
		}
	};

	// Sort windows according to their position
	QList<QMdiSubWindow*> subWindows = ui.mdiArea->subWindowList();
	qSort(subWindows.begin(), subWindows.end(), SubWindowOrder());

	QMdiArea::WindowOrder previousActivationOrder = ui.mdiArea->activationOrder();

	// Activate windows in order
	Q_FOREACH (QMdiSubWindow *pWindow, subWindows)
		ui.mdiArea->setActiveSubWindow(pWindow);

	// Tile windows arrocding to activation history
	ui.mdiArea->setActivationOrder(QMdiArea::ActivationHistoryOrder);
	ui.mdiArea->tileSubWindows();

	// Reset activation order
	ui.mdiArea->setActivationOrder(previousActivationOrder);
}

// Gets the main window plugin manager.
PluginManager<MainWindow*>& mainWindowPlugins()
{
	static PluginManager<MainWindow*> pluginManager;
	return pluginManager;
}
