#ifndef UI_H
#define UI_H

#include "breezEd.h"
#include <lean/tags/noncopyable.h>

class QWidget;

template <class Class>
struct UI : public lean::noncopyable, public Class
{
	LEAN_INLINE UI(QWidget *widget)
	{
		this->setupUi( LEAN_ASSERT_NOT_NULL(widget) );
	}

protected:
	using Class::setupUi;
	using Class::retranslateUi;
};

inline bool isChildOf(const QObject *child, const QObject *parent)
{
	while (child)
		if (child != parent)
			child = child->parent();
		else
			return true;

	return false;
}

template <class T>
inline T findParent(QObject *obj)
{
	while (obj)
	{
		obj = obj->parent();

		if (T parent = qobject_cast<T>(obj))
			return parent;
	}

	return nullptr;
}

template <class T>
inline T findParentInc(QObject *obj)
{
	while (obj)
	{
		if (T parent = qobject_cast<T>(obj))
			return parent;

		obj = obj->parent();
	}

	return nullptr;
}

#endif