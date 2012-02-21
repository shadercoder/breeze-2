/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMeshCompound.h"
#include "beScene/beMesh.h"

namespace beScene
{

// Constructor,
MeshCompound::MeshCompound(MeshCache *pCache)
	: m_pCache(pCache)
{
}

// Constructor,
MeshCompound::MeshCompound(const Mesh *const *pBegin, const Mesh *const *pEnd, MeshCache *pCache)
	: m_subsets(pBegin, pEnd),
	m_pCache(pCache)
{
}

// Destructor.
MeshCompound::~MeshCompound()
{
}

// Adds a subset.
void MeshCompound::AddSubset(const Mesh *pMesh)
{
	m_subsets.push_back(pMesh);
}

} // namespace