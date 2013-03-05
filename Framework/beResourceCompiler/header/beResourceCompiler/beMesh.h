/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#pragma once
#ifndef BE_RESOURCECOMPILER_MESH
#define BE_RESOURCECOMPILER_MESH

#include "beResourceCompiler.h"
#include <beCore/beShared.h>

namespace beResourceCompiler
{

/// Mesh class.
class Mesh : public beCore::OptionalResource
{
protected:
	Mesh& operator =(const Mesh&) { return *this; }

public:
	virtual ~Mesh() { }
};

}

#endif