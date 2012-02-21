#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QtCore/QObject.h>
#include <QtGui/QIcon.h>
#include <QtCore/QString.h>
#include <QtCore/QMap.h>

class AbstractDocument;
class AbstractDocumentFactory;

/// Document type description.
struct DocumentType
{
	/// Document type attributes enumeration.
	enum Attributes
	{
		DeleteFactoryOnShutdown = 0x01	///< Delete the factory on shut-down.
	};

	QString name;						///< Document type name.
	QString description;				///< Document type description.
	AbstractDocumentFactory *pFactory;	///< Document factory.
	QIcon icon;							///< Document icon.
	unsigned long attributes;			///< Document type attributes.

	/// Default constructor.
	DocumentType() : pFactory(nullptr), attributes(0) { };
	/// Constructor. Fully initializes this structure.
	DocumentType(const QString& name, const QString& description,
		AbstractDocumentFactory *pFactory, const QIcon &icon, unsigned long attributes = 0)
		: name(name), description(description),
		pFactory(pFactory), icon(icon), attributes(attributes) { };

	/// Checks whether this structure is valid.
	bool valid() const { return (!name.isEmpty() && pFactory != nullptr); };
};

/// Document manager class.
class DocumentManager : public QObject
{
private:
	typedef QMap<QString, DocumentType> document_type_map;
	document_type_map m_documentTypes;

	QObjectCleanupHandler m_cleanUpHandler;

public:
	/// Constructor.
	DocumentManager(QObject *pParent = nullptr);
	/// Destructor.
	~DocumentManager();

	/// Adds a document type.
	void addDocumentType(const DocumentType &documentType);
	/// Removes a document type.
	AbstractDocumentFactory* removeDocumentType(const QString &name);

	/// Gets a document type.
	DocumentType documentType(const QString &name) const { return m_documentTypes[name]; };
	/// Gets all document types.
	QList<DocumentType> documentTypes(void) const { return m_documentTypes.values(); };
};

#endif
