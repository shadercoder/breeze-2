/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#pragma once
#ifndef BE_RESOURCECOMPILER_SCENE
#define BE_RESOURCECOMPILER_SCENE

#include "beResourceCompiler.h"
#include <beCore/beShared.h>

namespace beResourceCompiler
{

/// Scene class.
class Scene : public beCore::OptionalResource
{
protected:
	Scene& operator =(const Scene&) { return *this; }

public:
	virtual ~Scene() { }
};

}

#endif