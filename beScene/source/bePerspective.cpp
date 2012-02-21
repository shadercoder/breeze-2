/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/bePerspective.h"

#include <beMath/beMatrix.h>
#include <beMath/beProjection.h>

namespace beScene
{

// Multiplies the given two matrices.
beMath::fmat4 PerspectiveDesc::Multiply(const beMath::fmat4 &a, const beMath::fmat4 &b)
{
	return mul(a, b);
}

// Extracts a view frustum from the given view projection matrix.
void PerspectiveDesc::ExtractFrustum(beMath::fplane3 frustum[6], const beMath::fmat4 &viewProj)
{
	extract_frustum(viewProj, frustum);
}

// Constructor.
Perspective::Perspective(const PerspectiveDesc &desc)
	: m_desc(desc),
	m_dataHeap(1024),
	m_dataHeapWatermark(0)
{
}

// Destructor.
Perspective::~Perspective()
{
}

// Allocates data from the perspective's internal heap.
void* Perspective::AllocateData(size_t size)
{
	m_dataHeapWatermark += size;
	return m_dataHeap.allocate(size);
}

// Frees all data allocated from this perspective's internal heap.
void Perspective::FreeData()
{
	size_t minDataCapacity = m_dataHeapWatermark;
	m_dataHeapWatermark = 0;

	m_dataHeap.clearButFirst();
	m_dataHeap.reserve(minDataCapacity);
}

} // namespace
