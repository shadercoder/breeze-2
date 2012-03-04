#include "stdafx.h"
#include "Documents/AbstractDocument.h"

#include "Utility/Checked.h"

// Constructor.
AbstractDocument::AbstractDocument(QString type, const QString &name, const QString &file, Editor *pEditor, QObject *pParent)
	: QObject(pParent),
	m_pEditor(LEAN_ASSERT_NOT_NULL(pEditor)),
	m_type(type),
	m_name( (!name.isEmpty()) ? name : tr("<Untitled>") ),
	m_file(file),
	m_bChanged(false),
	m_references(0),
	m_bClosingSignalEmitted(false)
{
}

// Destructor.
AbstractDocument::~AbstractDocument()
{
	maybeEmitClosingSignal(true);
}

// Saves the document.
bool AbstractDocument::save()
{
	return saveAs(file());
}

// Closes this document.
void AbstractDocument::close(void)
{
	maybeEmitClosingSignal(false);
	deleteLater();
}

// Emits the closing signal if not already done before.
void AbstractDocument::maybeEmitClosingSignal(bool bDestructed)
{
	if(!m_bClosingSignalEmitted)
	{
		// Absorb further closing signals
		m_bClosingSignalEmitted = true;

		// Inform listeners about closing document
		Q_EMIT documentClosing(this, bDestructed);
	}
}

// Sets the document name.
void AbstractDocument::setName(const QString &name)
{
	if(name != m_name)
	{
		m_name = name;
		Q_EMIT nameChanged(m_name);
	}
}

// Sets the document file.
void AbstractDocument::setFile(const QString &file)
{
	if(file != m_file)
	{
		m_file = file;
		Q_EMIT fileChanged(m_file);
	}
}

// Sets if the document has been changed.
void AbstractDocument::setChanged(bool bChanged)
{
	if(bChanged != m_bChanged)
	{
		m_bChanged = bChanged;
		Q_EMIT documentChanged(m_bChanged);
		
		if (!m_bChanged)
			Q_EMIT documentClean();
	}
}
