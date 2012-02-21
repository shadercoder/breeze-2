#include "./header/stdafx.h"
#include "./header/Documents/AbstractDocumentFactory.h"

// Constructor.
AbstractDocumentFactory::AbstractDocumentFactory(QObject *pParent)
	: QObject(pParent)
{
}

// Destructor.
AbstractDocumentFactory::~AbstractDocumentFactory()
{
}
