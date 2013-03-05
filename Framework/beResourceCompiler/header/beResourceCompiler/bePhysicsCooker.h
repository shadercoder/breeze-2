/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#pragma once
#ifndef BE_RESOURCECOMPILER_PHYSICSCOOKER
#define BE_RESOURCECOMPILER_PHYSICSCOOKER

#include "beResourceCompiler.h"
#include <lean/smart/scoped_ptr.h>

namespace beResourceCompiler
{

/// Physics cooker class.
class PhysicsCooker
{
public:
	struct Data;

private:
	lean::scoped_ptr<Data> m_data;

public:
	/// Constructor.
	BE_RESOURCECOMPILER_API PhysicsCooker();
	/// Destructor.
	BE_RESOURCECOMPILER_API ~PhysicsCooker();

	/// Gets the internal cooker data.
	LEAN_INLINE const Data& GetData() { return *m_data; }
};

}

#endif