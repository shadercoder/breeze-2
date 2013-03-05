#include "stdafx.h"
#include "Documents/CodeDocument.h"

#include <QtWidgets/QPlainTextDocumentLayout>

#include <QtCore/QFile.h>
#include <QtCore/QFileInfo.h>

#include "Editor.h"
#include "Windows/MainWindow.h"

#include "Utility/Checked.h"

// Code document type name.
const char *const CodeDocument::DocumentTypeName = "Code";

namespace
{

/// Sets up a plain text document.
void setupPlaintextDocument(QTextDocument &document)
{
	document.setDocumentLayout( new QPlainTextDocumentLayout(&document) );
	document.setDefaultFont( QFont("Courier New", 10) );
	document.setIndentWidth( QFontMetrics(document.defaultFont()).width(' ') * 4 );
}

} // namespace

// Constructor.
CodeDocument::CodeDocument(const QString &type, const QString &name, const QString &file, Editor *pEditor, QObject *pParent)
	: AbstractDocument(type, name, file, pEditor, pParent)
{
	setupPlaintextDocument(m_document);

	// Keep documents in sync
	checkedConnect(&m_document, SIGNAL(modificationChanged(bool)), this, SLOT(setChanged(bool)));
	checkedConnect(this, SIGNAL(documentChanged(bool)), &m_document, SLOT(setModified(bool)));
}

// Destructor.
CodeDocument::~CodeDocument()
{
}

// Saves the document using the given filename.
bool CodeDocument::saveAs(const QString &file)
{
	QFile textFile(file);

	if (textFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		textFile.write(m_document.toPlainText().toUtf8());
		textFile.flush();

		// Clean state
		setFile(file);
		setChanged(false);

		// Status update
		editor()->showMessage( AbstractDocument::tr("Saved document '%1' to '%2'.").arg(name()).arg(file) );

		return true;
	}
	else
		return false;
}

#include "Documents/AbstractDocumentFactory.h"
#include "Views/CodeView.h"
#include "Modes/Mode.h"
#include "Modes/DocumentModeState.h"
#include "Modes/CodeModeState.h"
#include "Modes/ParallelMode.h"

namespace
{

/// Code document factory class.
class CodeDocumentFactory : public AbstractDocumentFactory
{
public:
	/// Constructor.
	CodeDocumentFactory(QObject *pParent = nullptr)
		: AbstractDocumentFactory(pParent) { }
	/// Destructor.
	virtual ~CodeDocumentFactory() { }

	/// Creates a document.
	AbstractDocument* createDocument(const QString &name, const QString &file, Editor *pEditor, QObject *pParent)
	{
		return new CodeDocument(CodeDocument::DocumentTypeName, name, file, pEditor, pParent);
	}

	/// Builds a file path from the given name & directory.
	QString fileFromDir(const QString &name, const QString &path) const
	{
		QFileInfo fileInfo(path, name);
		QString absolutePath = fileInfo.absoluteFilePath();

		if (fileInfo.suffix().isEmpty())
			absolutePath += ".txt";

		return absolutePath;
	}

	/// Opens a document.
	AbstractDocument* openDocument(const QString &file, Editor *pEditor, QObject *pParent)
	{
		QFile textFile(file);

		if (textFile.open(QIODevice::ReadOnly))
		{
			QString name = QFileInfo(file).fileName();

			std::auto_ptr<CodeDocument> pDocument( static_cast<CodeDocument*>( createDocument(name, file, pEditor, pParent) ) );

			pDocument->textDocument()->setPlainText( QString::fromUtf8( textFile.readAll() ) ); 
			pDocument->setChanged(false);
			
			return pDocument.release();
		}
		
		return nullptr;
	}
	
	/// Checks if the given file may be opened using this document factory.
	FileMatch::T matchFile(const QString &file) const
	{
		return FileMatch::Any;
	}

	/// Creates a default mdi document view.
	QWidget* createDocumentView(AbstractDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	{
		DocumentModeState *pDocumentModeState = pDocumentMode->findChild<DocumentModeState*>();
		Mode *pViewModes = LEAN_ASSERT_NOT_NULL(pDocumentModeState)->viewModes();
		return new CodeView( qobject_cast<CodeDocument*>(pDocument), LEAN_ASSERT_NOT_NULL(pViewModes), pEditor, pParent, flags );
	}

	/// Creates a document mode.
	Mode* createDocumentMode(AbstractDocument *pDocument, Editor *pEditor, Mode *pParent)
	{
		std::auto_ptr<Mode> pMode( new Mode(pParent) );

		DocumentModeState *pDocumentModeState = new DocumentModeState(pDocument, pEditor, pMode.get());
		pMode->addState( pDocumentModeState );
		
		pMode->addState( new CodeModeState(qobject_cast<CodeDocument*>(pDocument), pEditor, pMode.get()) );
		
		pDocumentModeState->setViewModes( new Mode(pDocumentModeState) );
		pMode->addState( new ParallelMode(pDocumentModeState->viewModes(), pMode.get()) );
		
		return pMode.release();
	}
};

} // namespace

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Editor.h"
#include "Documents/DocumentManager.h"

namespace
{

/// Plugin class.
struct CodeDocumentPlugin : public AbstractPlugin<Editor*>
{
	mutable CodeDocumentFactory factory;

	/// Constructor.
	CodeDocumentPlugin()
	{
		editorPlugins().addPlugin(this);
	}

	/// Destructor.
	~CodeDocumentPlugin()
	{
		editorPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(Editor *pEditor) const
	{
		pEditor->documentManager()->addDocumentType(
				DocumentType(
					CodeDocument::DocumentTypeName,
					CodeDocument::tr("Plain text."),
					&factory,
					QIcon(":/breezEd/images/documentType/code")
				)
			);
	}
	/// Finalizes the plugin.
	void finalize(Editor *pEditor) const
	{
		pEditor->documentManager()->removeDocumentType(CodeDocument::DocumentTypeName);
	}
};

const CodeDocumentPlugin CodeDocumentPlugin;

} // namespace
