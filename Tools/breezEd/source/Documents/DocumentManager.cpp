#include "stdafx.h"
#include "Documents/DocumentManager.h"

#include "Documents/AbstractDocumentFactory.h"

#include "Utility/Strings.h"
#include <lean/logging/errors.h>

// Constructor.
DocumentManager::DocumentManager(QObject *pParent)
	: QObject(pParent)
{
}

// Destructor.
DocumentManager::~DocumentManager()
{
}

// Adds a document type
void DocumentManager::addDocumentType(const DocumentType &documentType)
{
	if(!documentType.valid())
	{
		LEAN_LOG_ERROR_CTX("Document type invalid", toUtf8Range(documentType.name));
		return;
	}

	m_documentTypes[documentType.name] = documentType;

	// Transfer ownership for automatic deletion, if requested
	if(documentType.attributes & DocumentType::DeleteFactoryOnShutdown)
		m_cleanUpHandler.add(documentType.pFactory);
}

// Removes a document type
AbstractDocumentFactory* DocumentManager::removeDocumentType(const QString &sName)
{
	AbstractDocumentFactory *pDocumentFactory = nullptr;

	document_type_map::iterator itDocumentType = m_documentTypes.find(sName);

	if(itDocumentType != m_documentTypes.end())
	{
		pDocumentFactory = itDocumentType->pFactory;
		bool bManagedFactory = itDocumentType->attributes & DocumentType::DeleteFactoryOnShutdown;
		
		m_documentTypes.erase(itDocumentType);

		// Transfer ownership back to caller
		if(bManagedFactory)
			m_cleanUpHandler.remove(pDocumentFactory);
	}

	return pDocumentFactory;
}
