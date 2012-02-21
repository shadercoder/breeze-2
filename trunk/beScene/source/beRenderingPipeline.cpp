/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderingPipeline.h"
#include "beScene/bePipelinePerspective.h"
#include "beScene/bePerspectiveModifier.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <lean/functional/algorithm.h>
#include <lean/logging/errors.h>

namespace beScene
{

/// Rendering Pipeline.
class RenderingPipeline::Impl
{
public:
	typedef std::vector<const Scenery*> scenery_vector;
	scenery_vector scenery;
	
	typedef boost::ptr_vector<PipelinePerspective> perspective_vector;
	perspective_vector perspectives;
	size_t activePerspectiveCount;

	uint4 maxPerspectiveCount;

	PerspectiveModifier *pPerspectiveModifier;

	/// Pipeline stage.
	struct Stage
	{
		utf8_string name;		///< Name.
		PipelineStageDesc desc;	///< Description.

		/// Constructor.
		Stage(const utf8_ntri &name,
			const PipelineStageDesc &desc)
				: name( name.to<utf8_string>() ),
				desc( desc ) { }
	};
	typedef std::vector<Stage> stage_vector;
	stage_vector stages;

	typedef std::vector<uint2> id_vector;
	id_vector sortedStageIDs;
	id_vector sortedStageSlots;

	PipelineStageMask normalStageMask;

	/// Render queue.
	struct Queue
	{
		utf8_string name;		///< Name.
		RenderQueueDesc desc;	///< Description.

		/// Constructor.
		Queue(const utf8_ntri &name,
			const RenderQueueDesc &desc)
				: name( name.to<utf8_string>() ),
				desc( desc ) { }
	};
	typedef std::vector<Queue> queue_vector;
	queue_vector queues;

	id_vector sortedQueueIDs;
	id_vector sortedQueueSlots;

	uint2 defaultStageID;
	uint2 defaultQueueIDs[MaxPipelineStageCount];

	/// Constructor.
	Impl()
		: activePerspectiveCount(0),
		pPerspectiveModifier(nullptr),
		normalStageMask(0),
		defaultStageID(InvalidPipelineStage),
		defaultQueueIDs(),
		maxPerspectiveCount(16) { }
};

// Constructor.
RenderingPipeline::RenderingPipeline(const utf8_ntri &name)
	: m_name(name.to<utf8_string>()),
	m_impl(new Impl())
{
}

// Destructor.
RenderingPipeline::~RenderingPipeline()
{
}

// Adds a perspective.
PipelinePerspective* RenderingPipeline::AddPerspective(const PerspectiveDesc &desc, Pipe *pPipe,
	PipelineProcessor *pProcessor, PipelineStageMask stageMask)
{
	if (m_impl->activePerspectiveCount >= m_impl->maxPerspectiveCount)
		LEAN_THROW_ERROR_MSG("Pipeline max perspective count exceeded!");

	PerspectiveDesc modifiedDesc(desc);

	// Allow for the adaption of perspectives to special GPU-only global transformations
	if (m_impl->pPerspectiveModifier)
		m_impl->pPerspectiveModifier->PerspectiveAdded(modifiedDesc);

	if (stageMask == 0)
		stageMask = m_impl->normalStageMask;

	PipelinePerspective *pPerspective;

	// Reset existing perspective, only create new one if none available
	if (m_impl->activePerspectiveCount == m_impl->perspectives.size())
	{
		pPerspective = new PipelinePerspective(this, modifiedDesc, pPipe, pProcessor, stageMask);
		m_impl->perspectives.push_back(pPerspective);
	}
	else
	{
		pPerspective = &m_impl->perspectives[m_impl->activePerspectiveCount];
		pPerspective->Reset(modifiedDesc, pPipe, pProcessor, stageMask);
	}

	++m_impl->activePerspectiveCount;

	return pPerspective;
}

// Clears all perspectives.
void RenderingPipeline::ClearPerspectives()
{
	for (size_t i = 0; i < m_impl->activePerspectiveCount; ++i)
		m_impl->perspectives[i].Release();

	m_impl->activePerspectiveCount = 0;

	// TODO: keep here?
	m_impl->scenery.clear();
}

// Releases shared references held.
void RenderingPipeline::Release()
{
	ClearPerspectives();
}

// Sets a perspective modifier (nullptr to unset), returning any previous perspective modifier.
PerspectiveModifier* RenderingPipeline::SetPerspectiveModifier(PerspectiveModifier *pModifier)
{
	if (pModifier)
		pModifier->ModifierAdded(this);

	PerspectiveModifier *pPreviousModifier = m_impl->pPerspectiveModifier;
	m_impl->pPerspectiveModifier = pModifier;

	if (pPreviousModifier)
		pPreviousModifier->ModifierRemoved(this);

	return pPreviousModifier;
}

// Includes all visible objects in the given scenery.
void RenderingPipeline::AddScenery(const Scenery &scenery)
{
	m_impl->scenery.push_back(&scenery);
}

// Prepares rendering of all stages and queues.
void RenderingPipeline::Prepare()
{
	// WARNING: activePerspectiveCount may change during loop
	for (size_t i = 0; i < m_impl->activePerspectiveCount; ++i)
	{
		// WARNING: perspective only valid inside this scope
		{
			PipelinePerspective &perspective = m_impl->perspectives[i];

			for (Impl::scenery_vector::const_iterator it = m_impl->scenery.begin(); it != m_impl->scenery.end(); ++it)
			{
				perspective.AddRenderables(**it);
				perspective.AddLights(**it);
			}

			// WARNING: This will invalidate the address of the current perspective when new perspectives are added
			perspective.Enqueue();
		}

		// WARNING: Always RE-FETCH reference to perspective here
		PipelinePerspective &perspective = m_impl->perspectives[i];

		perspective.Prepare();
		perspective.Optimize();
	}
}

// Prepares rendering of the given stage.
void RenderingPipeline::Prepare(uint2 stageID)
{
	// TODO: Missing enqueue!

	for (size_t i = 0; i < m_impl->activePerspectiveCount; ++i)
	{
		PipelinePerspective &perspective = m_impl->perspectives[i];
		perspective.Prepare(stageID);
		perspective.Optimize(stageID);
	}
}

// Renders all stages and queues.
void RenderingPipeline::Render(const RenderContext &context) const
{
	for (Impl::id_vector::const_iterator it = m_impl->sortedStageIDs.begin();
		it != m_impl->sortedStageIDs.end(); ++it)
		Render(*it, context);
}

// Renders the given stage and render queue.
void RenderingPipeline::Render(uint2 stageID, const RenderContext &context) const
{
	for (size_t i = m_impl->activePerspectiveCount; i-- > 0; )
	{
		const PipelinePerspective &perspective = m_impl->perspectives[i];

		if (perspective.GetStageMask() & (1 << stageID))
			perspective.Render(stageID, context);
	}
}

// Renders all stages and queues including the given scenery.
void RenderingPipeline::Render(const Scenery &scenery, const RenderContext &context)
{
	AddScenery(scenery);
	Prepare();
	Render(context);
}

namespace
{

/// Sorts structed elements by their names.
template <class Type>
struct NameAttributeCompare
{
	utf8_ntr name;

	NameAttributeCompare(const utf8_ntr &name)
		: name(name) { }

	LEAN_INLINE bool operator ()(const Type &elem) { return (elem.name == name); }
};

/// Sorts IDs to structed elements by their description layer attributes.
template <class Element>
struct IDByLayerCompare
{
	const Element *elements;

	IDByLayerCompare(const Element *elements)
		: elements(elements) { }

	LEAN_INLINE bool operator ()(uint2 left, uint2 right) { return (elements[left].desc.Layer < elements[right].desc.Layer); }
};

} // namespace

// Adds a pipeline stage according to the given description.
uint2 RenderingPipeline::AddStage(const utf8_ntri &stageName, const PipelineStageDesc &desc)	
{
	Impl::stage_vector &stages = m_impl->stages;
	Impl::stage_vector::iterator it = std::find_if(
		stages.begin(), stages.end(),
		NameAttributeCompare<Impl::Stage>(make_ntr(stageName)) );

	// Enforce unique names
	if (it == stages.end())
	{
		if (it == stages.end())
			it = stages.insert(stages.end(), Impl::Stage(stageName, desc));
		else
			*it = Impl::Stage(stageName, desc);

		// Insert stage into ordered rendering sequence
		uint2 stageID = static_cast<uint2>(it - stages.begin());
		Impl::id_vector &stageIDs = m_impl->sortedStageIDs;
		lean::push_sorted(stageIDs, 
			stageID,
			IDByLayerCompare<Impl::Stage>(&stages[0]) );

		if (desc.Normal)
			m_impl->normalStageMask |= 1 << stageID;

		// Rebuild stage slot table
		Impl::id_vector &stageSlots = m_impl->sortedStageSlots;
		stageSlots.resize(stages.size(), static_cast<uint2>(-1));
		for (Impl::id_vector::const_iterator it = stageIDs.begin(); it != stageIDs.end(); ++it)
			stageSlots[*it] = static_cast<uint2>(it - stageIDs.begin());
	}
	// Warn about mismatching adds
	else if(memcmp(&it->desc, &desc, sizeof(desc)) != 0)
		LEAN_LOG_ERROR_CTX("Stage added twice, descriptions mismatching", stageName.c_str());

	return static_cast<uint2>(it - stages.begin());
}

// Adds a render queue according to the given description.
uint2 RenderingPipeline::AddQueue(const utf8_ntri &queueName, const RenderQueueDesc &desc)
{
	Impl::queue_vector &queues = m_impl->queues;
	Impl::queue_vector::iterator it = std::find_if(
		queues.begin(), queues.end(), 
		NameAttributeCompare<Impl::Queue>(make_ntr(queueName)) );

	// Enforce unique names
	if (it == queues.end())
	{
		if (it == queues.end())
			it = queues.insert(queues.end(), Impl::Queue(queueName, desc));
		else
			*it = Impl::Queue(queueName, desc);

		// Insert queue into ordered rendering sequence
		uint2 queueID = static_cast<uint2>(it - queues.begin());
		Impl::id_vector &queueIDs = m_impl->sortedQueueIDs;
		lean::push_sorted(m_impl->sortedQueueIDs, 
			queueID,
			IDByLayerCompare<Impl::Queue>(&queues[0]) );

		// Rebuild queue slot table
		Impl::id_vector &queueSlots = m_impl->sortedQueueSlots;
		queueSlots.resize(queues.size(), static_cast<uint2>(-1));
		for (Impl::id_vector::const_iterator it = queueIDs.begin(); it != queueIDs.end(); ++it)
			queueSlots[*it] = static_cast<uint2>(it - queueIDs.begin());
	}
	// Warn about mismatching adds
	else if(memcmp(&it->desc, &desc, sizeof(desc)) != 0)
		LEAN_LOG_ERROR_CTX("Queue added twice, descriptions mismatching", queueName.c_str());

	return static_cast<uint2>(it - queues.begin());
}

// Sets a new setup for the given pipeline stage.
void RenderingPipeline::SetStageSetup(uint2 stageID, const QueueSetup *pSetup)
{
	LEAN_ASSERT(stageID < m_impl->stages.size());

	m_impl->stages[stageID].desc.Setup = pSetup;
}

// Sets a new setup for the given render queue.
void RenderingPipeline::SetQueueSetup(uint2 queueID, const QueueSetup *pSetup)
{
	LEAN_ASSERT(queueID < m_impl->queues.size());

	m_impl->queues[queueID].desc.Setup = pSetup;
}

// Gets the ID of the pipeline stage identified by the given name.
uint2 RenderingPipeline::GetStageID(const utf8_ntri &stageName) const
{
	Impl::stage_vector::const_iterator it = std::find_if(
		m_impl->stages.begin(), m_impl->stages.end(),
		NameAttributeCompare<Impl::Stage>(make_ntr(stageName)) );

	return (it != m_impl->stages.end())
		? static_cast<uint2>(it - m_impl->stages.begin())
		: InvalidPipelineStage;
}

// Gets the ID of the render queue identified by the given name.
uint2 RenderingPipeline::GetQueueID(const utf8_ntri &queueName) const
{
	Impl::queue_vector::const_iterator it = std::find_if(
		m_impl->queues.begin(), m_impl->queues.end(), 
		NameAttributeCompare<Impl::Queue>(make_ntr(queueName)) );

	return (it != m_impl->queues.end())
		? static_cast<uint2>(it - m_impl->queues.begin())
		: InvalidRenderQueue;
}

// Gets the number of pipeline stages.
uint2 RenderingPipeline::GetStageCount() const
{
	return static_cast<uint2>(m_impl->stages.size());
}

// Gets the number of render queues.
uint2 RenderingPipeline::GetQueueCount() const
{
	return static_cast<uint2>(m_impl->queues.size());
}

// Gets the description of the given pipeline stage.
const PipelineStageDesc& RenderingPipeline::GetStageDesc(uint2 stageID) const
{
	LEAN_ASSERT(stageID < m_impl->stages.size());
	return m_impl->stages[stageID].desc;
}

// Gets the description of the given pipeline stage.
const RenderQueueDesc& RenderingPipeline::GetQueueDesc(uint2 queueID) const
{
	LEAN_ASSERT(queueID < m_impl->queues.size());
	return m_impl->queues[queueID].desc;
}

// Gets an ordered sequence of pipeline stage IDs.
const uint2* RenderingPipeline::GetOrderedStageIDs() const
{
	return &m_impl->sortedStageIDs[0];
}

// Gets a pipeline stage slot lookup table.
const uint2* RenderingPipeline::GetStageSlots() const
{
	return &m_impl->sortedStageSlots[0];
}

// Gets an ordered sequence of render queue IDs.
const uint2* RenderingPipeline::GetOrderedQueueIDs() const
{
	return &m_impl->sortedQueueIDs[0];
}

// Gets a render queue slot lookup table.
const uint2* RenderingPipeline::GetQueueSlots() const
{
	return &m_impl->sortedQueueSlots[0];
}

// Gets the ID of the default pipeline stage.
void RenderingPipeline::SetDefaultStageID(uint2 stageID)
{
	if (stageID < m_impl->stages.size())
		m_impl->defaultStageID = stageID;
}

// Gets the ID of the default pipeline stage.
uint2 RenderingPipeline::GetDefaultStageID() const
{
	return m_impl->defaultStageID;
}

// Gets the ID of the default pipeline stage.
void RenderingPipeline::SetDefaultQueueID(uint2 stageID, uint2 queueID)
{
	if (stageID < m_impl->stages.size() && queueID < m_impl->queues.size())
		m_impl->defaultQueueIDs[stageID] = queueID;
}

// Gets the ID of the default pipeline stage.
uint2 RenderingPipeline::GetDefaultQueueID(uint2 stageID) const
{
	return (stageID < m_impl->stages.size())
		? m_impl->defaultQueueIDs[stageID]
		: InvalidRenderQueue;
}

// Sets the maximum number of active perspectives.
void RenderingPipeline::SetMaxPerspectiveCount(uint4 count)
{
	m_impl->maxPerspectiveCount = count;
}

// Gets the maximum number of active perspectives.
uint4 RenderingPipeline::GetMaxPerspectiveCount() const
{
	return m_impl->maxPerspectiveCount;
}

// Sets the name.
void RenderingPipeline::SetName(const utf8_ntri &name)
{
	m_name.assign(name.begin(), name.end());
}

} // namespace
