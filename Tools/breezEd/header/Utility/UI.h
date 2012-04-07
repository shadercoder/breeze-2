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

#endif