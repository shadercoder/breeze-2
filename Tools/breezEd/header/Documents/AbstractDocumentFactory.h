#ifndef ABSTRACTDOCUMENTFACTORY_H
#define ABSTRACTDOCUMENTFACTORY_H

#include <QtCore/QObject>

// Prototypes
class Editor;
class AbstractDocument;
class Mode;
class QWidget;

/// File match enumeration.
namespace FileMatch
{
	/// Enumeration
	enum T
	{
		Match,
		Mismatch,
		Any
	};
}

/// Abstract document factory class
class AbstractDocumentFactory : public QObject
{
	Q_OBJECT

public:
	/// Constructor.
	AbstractDocumentFactory(QObject *pParent = nullptr)
		: QObject(pParent) { }
	/// Destructor.
	virtual ~AbstractDocumentFactory() { };

	/// Creates a document.
	virtual AbstractDocument* createDocument(const QString &name, const QString &file, Editor *pEditor, QObject *pParent = nullptr) = 0;
	/// Builds a file path from the given name & directory.
	virtual QString fileFromDir(const QString &name, const QString &path) const = 0;
	/// Opens a document.
	virtual AbstractDocument* openDocument(const QString &file, Editor *pEditor, QObject *pParent = nullptr) = 0;
	/// Checks if the given file may be opened using this document factory.
	virtual FileMatch::T matchFile(const QString &file) const = 0;
	/// Creates a default mdi document view.
	virtual QWidget* createDocumentView(AbstractDocument *pDocument, Mode *pDocumentMode, Editor *pEditor,
		QWidget *pParent = nullptr, Qt::WFlags flags = 0) = 0;
	/// Creates a document mode.
	virtual Mode* createDocumentMode(AbstractDocument *pDocument, Editor *pEditor, 
		Mode *pParent = nullptr) = 0;
};

#endif
