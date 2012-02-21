/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/beSceneImpl.h"

namespace beResourceCompiler
{

// Constructor.
SceneImpl::SceneImpl(const aiScene *pScene)
	: m_pScene(pScene)
{
	// ASSERT: Flat scene
}

// Destructor.
SceneImpl::~SceneImpl()
{
}

} // namespace
