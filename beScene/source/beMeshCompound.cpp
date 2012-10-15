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

// Computes the combined bounds of the given mesh compound.
beMath::faab3 ComputeBounds(const Mesh *const *meshes, uint4 meshCount)
{
	beMath::faab3 result(beMath::faab3::invalid);
	const Mesh *const *meshesEnd = meshes + meshCount;

	while (meshes != meshesEnd)
	{
		const beMath::faab3 &box = (*meshes)->GetBounds();

		result.min = min_cw(box.min, result.min);
		result.max = max_cw(box.max, result.max);
		
		++meshes;
	}

	return result;
}

} // namespace