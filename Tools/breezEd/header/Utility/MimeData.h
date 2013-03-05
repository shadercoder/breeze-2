#ifndef MIMEDATA_H
#define MIMEDATA_H

#include "breezEd.h"
#include <QtCore/QMimeData>

/// Stores a pointer.
template <class T>
inline QMimeData* ptrToMimeData(const QString &type, T *ptr)
{
	QMimeData *data = new QMimeData();
	data->setData(type,  QByteArray::number(reinterpret_cast<uintptr_t>(ptr), 16));
	return data;
}

// Retrieves a pointer.
template <class T>
inline T* ptrFromMimeData(const QString &type, const QMimeData *pData, T *defaultPtr = nullptr)
{
	T *ptr = defaultPtr;

	if (pData)
	{
		bool ok = false;
		LEAN_STATIC_ASSERT(sizeof(uintptr_t) <= sizeof(qulonglong));
		uintptr_t uiptr = static_cast<uintptr_t>( pData->data(type).toULongLong(&ok, 16) );
		if (ok)
			ptr = reinterpret_cast<T*>(uiptr);
	}

	return ptr;
}

#endif