/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beFlatScenery.h"
#include "beScene/beRenderable.h"
#include "beScene/beRenderJob.h"

#include <beMath/bePlane.h>
#include <beMath/beSphere.h>

#include <lean/functional/algorithm.h>

namespace beScene
{

/// Scenery element
struct FlatScenery::Element
{
	Renderable *pRenderable;
	RenderableData data;

	/// Constructor.
	explicit Element(Renderable *pRenderable)
		: pRenderable(pRenderable) { }
};

namespace
{

const size_t RenderableDataChunkSize = 256 * 1024;
const size_t PassDataChunkSize = 256 * 1024;

/// Orders renderables by their sort index.
struct RenderableOrder
{
	bool operator ()(Renderable *left, Renderable *right)
	{
		return left->GetSortIndex() < right->GetSortIndex();
	}
};

/// Orders passes by their stage, queue and sort index.
struct PassOrder
{
	typedef std::pair<uint4, uint4> value_type;

	const FlatScenery::Element *renderables;

	explicit PassOrder(const FlatScenery::Element *renderables)
		: renderables(renderables) { }

	bool operator ()(const value_type &left, const value_type &right)
	{
		const RenderablePass &leftPass = renderables[left.first].data.Passes[left.second];
		const RenderablePass &rightPass = renderables[right.first].data.Passes[right.second];

		if (leftPass.StageID < rightPass.StageID)
			return true;
		else if (leftPass.StageID == rightPass.StageID)
		{
			if (leftPass.QueueID < rightPass.QueueID)
				return true;
			else if (leftPass.QueueID == rightPass.QueueID)
				return (leftPass.SortIndex < rightPass.SortIndex);
		}

		return false;
	}
};

} // namespace

// Constructor.
FlatScenery::FlatScenery()
	: m_renderableData(RenderableDataChunkSize),
	m_passData(PassDataChunkSize),
	m_bRenderablesChanged(false)
{
}

// Destructor.
FlatScenery::~FlatScenery()
{
}

// Gets all renderables inside the given frustum.
void FlatScenery::CullRenderables(const beMath::fplane3 *planes, AddRenderablePassFunction *pAddPass, void *pCaller) const
{
	for (element_vector::const_iterator itElement = m_elements.begin(); itElement != m_elements.end(); ++itElement)
	{
		const RenderableData &data = itElement->data;
		
		bool renderableVisible = true;
		
		// Cull bounding sphere against frustum
		for (int i = 0; i < 6; ++i)
			renderableVisible &= ( sdist(planes[i], data.Bounds.p()) <= data.Bounds.r() );

		// Insert passes, if visible
		if (renderableVisible)
			(*pAddPass)(pCaller, itElement->pRenderable, data);
	}
}

// Gets all lights inside the given frustum.
void FlatScenery::CullLights(const beMath::fsphere3 &bounds, AddLightJobFunction *pAddLight, void *pCaller) const
{
	// TODO: Loop over lights
}

namespace
{

/// Heaps that allow for the consecutive storage of renderable data.
struct RenderableDataHeaps : public RenderableDataAllocator
{
	FlatScenery::data_heap renderableData;
	FlatScenery::data_heap passData;

	/// Constructor.
	RenderableDataHeaps()
		: renderableData(RenderableDataChunkSize),
		passData(PassDataChunkSize) { }

	/// Allocates data from the shared renderable heap.
	void* AllocateRenderableData(size_t size)
	{
		return renderableData.allocate(size);
	}

	/// Allocates data from the renderable pass heap.
	void* AllocateRenderablePassData(size_t size)
	{
		return passData.allocate(size);
	}
};

} // namespace

// Prepares the scenery for rendering access.
void FlatScenery::Prepare()
{
	if (m_bRenderablesChanged)
	{
		// Pre-sort renderables to improve renderable data cache coherence
		std::sort(m_renderables.begin(), m_renderables.end(), RenderableOrder());

		element_vector elements(m_renderables.size());
		RenderableDataHeaps renderableHeaps;

		uint4 passCount = 0;

		for (renderable_vector::const_iterator itRenderable = m_renderables.begin(); itRenderable != m_renderables.end(); ++itRenderable)
		{
			Renderable *pRenderable = *itRenderable;

			// Insert element into scenery
			Element &element = elements.emplace_back(pRenderable);
			
			// Pre-allocate passes
			element.data.PassCount = pRenderable->GetPassCount();
			
			if (element.data.PassCount > 0)
			{
				element.data.Passes = new ( renderableHeaps.passData.allocate(sizeof(RenderablePass) * element.data.PassCount) ) RenderablePass[element.data.PassCount];
				passCount += element.data.PassCount;
			}

			// (Re-)attach renderable to scenery, particularly relocate renderable data
			pRenderable->Attached(this, element.data, renderableHeaps);

			LEAN_ASSERT_NOT_NULL(element.data.Render);
		}

		typedef lean::dynamic_array<PassOrder::value_type> pass_vector;
		pass_vector passes(passCount);

		// Collect indices to passes
		for (element_vector::const_iterator itElement = elements.begin(); itElement != elements.end(); ++itElement)
		{
			const RenderableData &renderable = itElement->data;
			uint4 renderableIdx = static_cast<uint4>( itElement - elements.begin() );

			for (uint4 i = 0; i < renderable.PassCount; ++i)
				passes.push_back( PassOrder::value_type(renderableIdx, i) );
		}

		// Pre-sort passes to improve pass data cache coherence
		std::sort(passes.begin(), passes.end(), PassOrder(&elements[0]));

		for (pass_vector::const_iterator itPass = passes.begin(); itPass != passes.end(); ++itPass)
		{
			Element &element = elements[itPass->first];
			RenderablePass &pass = element.data.Passes[itPass->second];

			// (Re-)attach renderable to scenery, particularly relocate pass data
			element.pRenderable->Attached(this, element.data, pass, itPass->second, renderableHeaps);
		}

		swap(elements, m_elements);
		swap(renderableHeaps.renderableData, m_renderableData);
		swap(renderableHeaps.passData, m_passData);
		m_bRenderablesChanged = false;
	}
}

// Adds the given render job to this scenery.
void FlatScenery::AddRenderable(Renderable *pRenderable)
{
	m_renderables.push_back(pRenderable);
	m_bRenderablesChanged = true;
}

// Removes the given renderable from this scenery.
void FlatScenery::RemoveRenderable(Renderable *pRenderable)
{
	if (lean::remove(m_renderables, pRenderable))
		pRenderable->Detached(this);

	m_bRenderablesChanged = true;
}

// Invalidates cached scenery data.
void FlatScenery::InvalidateRenderables()
{
	m_bRenderablesChanged = true;
}

// Adds the given light to this scenery.
void FlatScenery::AddLight(Light *pLight)
{
	// TODO
}

// Removes the given renderable from this scenery.
void FlatScenery::RemoveLight(Light *pLight)
{
	// TODO
}

// Invalidates cached scenery data.
void FlatScenery::InvalidateLights()
{
	// TODO
}

} // namespace
