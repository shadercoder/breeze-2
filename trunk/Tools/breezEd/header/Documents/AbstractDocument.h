#ifndef ABSTRACTDOCUMENT_H
#define ABSTRACTDOCUMENT_H

#include "breezEd.h"
#include <QtCore/QObject>

class Editor;
class QUndoStack;

/// Abstract document class
class AbstractDocument : public QObject
{
	Q_OBJECT

private:
	Editor *m_pEditor;

	QString m_type;
	QString m_name;
	QString m_file;
	
	bool m_bChanged;

	int m_references;
	bool m_bClosingSignalEmitted;

	/// Emits the closing signal if not already done.
	void maybeEmitClosingSignal(bool bDestructed);

protected Q_SLOTS:
	/// Sets the document file.
	void setFile(const QString &file);

public:
	/// Constructor.
	AbstractDocument(QString sType, const QString &name, const QString &file, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~AbstractDocument();

	/// Closes this document.
	void close();

	/// Gets the document type.
	LEAN_INLINE QString type() const { return m_type; }
	/// Gets the document name.
	LEAN_INLINE QString name() const { return m_name; }
	/// Gets the document file.
	LEAN_INLINE QString file() const { return m_file; }
	/// Gets if the document has been changed.
	LEAN_INLINE bool changed() const { return m_bChanged; }
	/// Gets the reference count of this document.
	LEAN_INLINE int references() const { return m_references; }

	/// Gets an undo stack.
	virtual QUndoStack* undoStack() { return nullptr; }
	/// Gets an undo stack.
	virtual const QUndoStack* undoStack() const { return nullptr; }

	/// Gets the editor.
	LEAN_INLINE Editor* editor() const { return m_pEditor; }

public Q_SLOTS:
	/// Saves the document.
	virtual bool save();
	/// Saves the document using the given filename.
	virtual bool saveAs(const QString &file) = 0;

	/// Sets the document name.
	virtual void setName(const QString &name);
	/// Sets if the document has been changed.
	virtual void setChanged(bool bChanged = true);
	/// Sets if the document has been changed.
	LEAN_INLINE void setClean(bool bClean = true) { setChanged(!bClean); }

	/// Increases the reference count of this document.
	LEAN_INLINE void addReference() { ++m_references; };
	/// Decreases the reference count of this document.
	LEAN_INLINE void removeReference() { --m_references; if (m_references < 1) close(); };

Q_SIGNALS:
	/// The document has been modified in a non-local way.
	void refresh();

	/// The name has changed.
	void nameChanged(const QString &name);
	/// The file has changed.
	void fileChanged(const QString &file);
	/// The document has either been changed or saved.
	void documentChanged(bool bChanged);
	/// The document has either been unchanged or saved.
	void documentClean();

	/// The document is about to be closed.
	void documentClosing(AbstractDocument *pDocument, bool bDestructed);
};

/// Document reference utility class
template <class T>
class DocumentReference
{
private:
	// Document pointer
	T *m_pDocument;

public:
	/// Default constructor. Sets this document reference to nullptr.
	DocumentReference() : m_pDocument(nullptr) { };
	/// Binding Constructor. Binds a reference of the given document to this document reference.
	DocumentReference(T* pDocument, bool bBind = false)
		: m_pDocument(pDocument)
	{
		if(m_pDocument && !bBind)
			m_pDocument->addReference();
	}
	/// Copy constructor. Binds a new reference of the document bound by the given
	/// document reference to this document reference.
	DocumentReference(const DocumentReference& docRef)
		: m_pDocument(docRef.m_pDocument)
	{
		if(m_pDocument)
			m_pDocument->addReference();
	}
	/// Destructor. Releases the reference of the document bound to this document reference.
	~DocumentReference()
	{
		if(m_pDocument)
			m_pDocument->removeReference();
	}

	/// Binds a new reference of the document bound by the given
	/// document reference to this document reference.
	DocumentReference& operator =(const DocumentReference& docRef)
	{
		return (*this = dpcRef.m_pDocument);
	}
	/// Binds a new reference of the given document to this document reference.
	DocumentReference& operator =(T* pDocument)
	{
		if(pDocument != m_pDocument)
		{
			if(pDocument)
				pDocument->addReference();
			
			T *pPrevDocument = m_pDocument;
			m_pDocument = pDocument;

			if(pPrevDocument)
				pPrevDocument->removeReference();
		}

		return *this;
	}

	/// Binds the given document to this document reference without incrementing its reference counter.
	void bind(T* pDocument)
	{
		T *pPrevDocument = m_pDocument;
		m_pDocument = pDocument;

		if(pPrevDocument)
			pPrevDocument->removeReference();
	}
	/// Unbinds the document bound by this document reference without decrementing its reference counter.
	T* unbind(void)
	{
		T *pDocument = m_pDocument;
		m_pDocument = nullptr;
		return pDocument;
	}

	/// Gets the document bound by this document reference.
	inline T* get() const { return m_pDocument; };
	/// Gets the document bound by this document reference.
	inline T& operator *() const { return *get(); };
	/// Gets the document bound by this document reference.
	inline T* operator ->() const { return get(); };
	/// Gets the document bound by this document reference.
	inline operator T*() const { return get(); };
};

#endif
