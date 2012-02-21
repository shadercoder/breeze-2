#ifndef CODEDOCUMENT_H
#define CODEDOCUMENT_H

#include "breezEd.h"
#include "AbstractDocument.h"

#include <QtGui/QTextDocument>

/// Code document class
class CodeDocument : public AbstractDocument
{
	Q_OBJECT

private:
	QTextDocument m_document;

public:
	/// Code document type name.
	static const char *const DocumentTypeName;

	/// Constructor.
	CodeDocument(const QString &type, const QString &file, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~CodeDocument();

	/// Saves the document using the given filename.
	virtual bool saveAs(const QString &file);

	/// Gets the text document.
	QTextDocument* textDocument() { return &m_document; }
	/// Gets the text document.
	const QTextDocument* textDocument() const { return &m_document; }
};

#endif
