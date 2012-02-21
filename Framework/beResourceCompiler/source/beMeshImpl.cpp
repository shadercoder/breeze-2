/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/beMeshImpl.h"

namespace beResourceCompiler
{

// Constructor.
MeshImpl::MeshImpl(const aiScene *pScene)
	: m_pScene(pScene)
{
	// ASSERT: Flat scene
}

// Destructor.
MeshImpl::~MeshImpl()
{
}

} // namespace
