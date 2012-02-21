/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/bePipelinePerspective.h"
#include "beScene/beScenery.h"
#include "beScene/beRenderable.h"
#include "beScene/bePipe.h"
#include "beScene/bePipelineProcessor.h"
#include <beCore/beVectorQueryResult.h>
#include "beScene/beRenderingPipeline.h"

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

namespace beScene
{

namespace
{

/// Reinterprets a value of the given source type as a value of the given destination type.
template <class Dest, class Src>
Dest reinterpret_value(const Src &src)
{
	LEAN_STATIC_ASSERT_MSG_ALT(
		sizeof(Dest) == sizeof(Src),
		"Cannot reinterpret values of different sizes.",
		Cannot_reinterpret_values_of_different_sizes );

	union Src2Dest
	{
		Src src;
		Dest dest;
		char a[sizeof(Dest)];
	};

	return reinterpret_cast<const Src2Dest&>(src).dest;
}

struct EnqueueRenderablePassesArguments
{
	PipelinePerspective *perspective;
	const PerspectiveDesc *perspectiveDesc;
	PipelinePerspective::Renderables *renderables;
	RenderingPipeline *pipeline;

	explicit EnqueueRenderablePassesArguments(PipelinePerspective *perspective,
		const PerspectiveDesc &perspectiveDesc,
		PipelinePerspective::Renderables &renderables,
		RenderingPipeline *pipeline)
			: perspective(perspective),
			perspectiveDesc(&perspectiveDesc),
			renderables(&renderables),
			pipeline(pipeline) {}
};

/// Enques the given list of renderable passes.
LEAN_INLINE void EnqueueRenderablePasses(void *pCaller, const Renderable *pRenderable, const RenderableData &renderableData)
{
	const EnqueueRenderablePassesArguments &args = *static_cast<const EnqueueRenderablePassesArguments*>(pCaller);
	
	// Compute renderable depth
	uint4 depth = reinterpret_value<uint4>( distSq(renderableData.Bounds.p(), args.perspectiveDesc->CamPos) );

	PipelinePerspective::Renderables &renderables = *args.renderables;
	void *pPerspectiveData = nullptr;

	// Used to ensure renderable is only prepared once
	uint4 preparePerspectiveMask = RenderableFlags::PerspectivePrepare;

	for (uint4 passIdx = 0; passIdx < renderableData.PassCount; ++passIdx)
	{
		const RenderablePass &pass = renderableData.Passes[passIdx];

		// Reject passes that have not been requested to be rendered
		if (renderables.stageMask & (1 << pass.StageID))
		{
			PipelinePerspective::Stage &stage = renderables.stages[pass.StageID];
			PipelinePerspective::Queue &queue = stage.queues[pass.QueueID];

			// Insert job into queue
			PipelinePerspective::QueuedRenderJob &renderJob = queue.renderJobs.push_back();
			renderJob.sortIndex = pass.SortIndex;
			renderJob.depth = depth;
			renderJob.data.Render = renderableData.Render;
			renderJob.data.SharedData = renderableData.SharedData;
			renderJob.data.PassData = pass.PassData;

			// Perform custom renderable setup
			if (pass.Flags & preparePerspectiveMask)
			{
				pPerspectiveData = pRenderable->Prepare(*args.perspective, *args.pipeline);

				if (pass.Flags & RenderableFlags::PerspectiveFinalize)
				{
					PipelinePerspective::RenderablePerspectiveData &finalizeData = renderables.perspectiveData.push_back();
					finalizeData.renderable = pRenderable;
					finalizeData.pPerspectiveData = pPerspectiveData;
				}

				// Do not prepare twice
				preparePerspectiveMask = 0;
			}
			renderJob.data.PerspectiveData = pPerspectiveData;

			// TODO: Light queries
		}
	}
}

struct AddLightArguments
{
	PipelinePerspective::light_vector *lights;

	explicit AddLightArguments(PipelinePerspective::light_vector &lights)
			: lights(&lights) {}
};

/// Adds the given light.
void AddLight(void *pCaller, const Light *pLight, const LightJob &lightJob)
{
	const AddLightArguments &args = *static_cast<const AddLightArguments*>(pCaller);

	// TODO: ????
	args.lights->push_back(pLight);
}

} // namespace

// Constructor.
PipelinePerspective::PipelinePerspective(RenderingPipeline *pPipeline, const PerspectiveDesc &desc, Pipe *pPipe, PipelineProcessor *pProcessor, PipelineStageMask stageMask)
	: Perspective(desc),
	m_pPipeline( LEAN_ASSERT_NOT_NULL(pPipeline) ),
	m_renderables( stageMask ),
	m_pPipe( pPipe ),
	m_pProcessor( pProcessor )
{
}

// Destructor.
PipelinePerspective::~PipelinePerspective()
{
}

// Sets the perspective description.
void PipelinePerspective::Reset(const PerspectiveDesc &desc, Pipe *pPipe, PipelineProcessor *pProcessor, PipelineStageMask stageMask)
{
	Dequeue();

	m_desc = desc;
	m_pPipe = pPipe;
	m_pProcessor = pProcessor;
	m_renderables.stageMask = stageMask;
}

// Releases resource references held.
void PipelinePerspective::Release()
{
	Dequeue();

	if (m_pPipe)
	{
		m_pPipe->Release();
		m_pPipe = nullptr;
	}

	m_pProcessor = nullptr;
}

// Adds all visible renderables in the given scenery to this perspective.
void PipelinePerspective::AddRenderables(const Scenery& scenery)
{
	// TODO: Disallow modifications after enqueuing!

	EnqueueRenderablePassesArguments perspectiveData(this, m_desc, m_renderables, m_pPipeline);
	scenery.CullRenderables(m_desc.Frustum, &EnqueueRenderablePasses, &perspectiveData);
}

// Adds the given renderable to this perspective.
void PipelinePerspective::AddRenderables(const Renderable *pRenderable, const RenderableData &renderableData)
{
	// TODO: Disallow modifications after enqueuing!

	EnqueueRenderablePassesArguments perspectiveData(this, m_desc, m_renderables, m_pPipeline);
	EnqueueRenderablePasses(&perspectiveData, pRenderable, renderableData);
}

// Adds all visible lights in the given scenery to this perspective.
void PipelinePerspective::AddLights(const Scenery& scenery)
{
/*	TODO: WHATEVER
	AddLightArguments perspectiveData(m_lights);
	scenery.CullLights(???, &beScene::AddLight, &perspectiveData);
*/
}

// Adds the given light to this perspective.
void PipelinePerspective::AddLight(const Light *pLight)
{
	LEAN_ASSERT(pLight != nullptr);

/*	TODO: WHATEVER
	m_lights.push_back(pLight);
*/
}

// Queues all renderables into their render queues and pipeline stages
void PipelinePerspective::Enqueue()
{
	const uint2 stageCount = m_pPipeline->GetStageCount();
	const uint2 queueCount = m_pPipeline->GetQueueCount();

	// Cache stage & queue order
	const uint2 *stageSlots = m_pPipeline->GetStageSlots();
	const uint2 *queueSlots = m_pPipeline->GetQueueSlots();

	uint2 minStageSlot = MaxPipelineStageCount;
	uint2 maxStageSlot = 0;

	for (uint2 stageID = 0; stageID < stageCount; ++stageID)
	{
		Stage &stage = m_renderables.stages[stageID];

		// Reject stages that have not been requested to be rendered
		if (m_renderables.stageMask & (1 << stageID))
		{
			for (uint2 queueID = 0; queueID < queueCount; ++queueID)
			{
				Queue &queue = stage.queues[queueID];

				// Ingore empty queues
				if (!queue.renderJobs.empty())
				{
					if (queueID < stage.minQueueID)
						stage.minQueueID = queueID;
					if (queueID > stage.maxQueueID)
						stage.maxQueueID = queueID;

					// Order queues
					uint2 queueSlot = queueSlots[queueID];

					if (queueSlot < stage.minQueueSlot)
					{
						stage.minQueueSlot = queueSlot;
						stage.firstQueueID = queueID;
					}
					// NOTE: Inclusive as initialized 0
					if (queueSlot >= stage.maxQueueSlot)
					{
						stage.maxQueueSlot = queueSlot;
						stage.lastQueueID = queueID;
					}
				}
			}

			// Ignore empty stages
			if (stage.minQueueID <= stage.maxQueueID)
			{
				if (stageID < m_renderables.minStageID)
					m_renderables.minStageID = stageID;
				if (stageID > m_renderables.maxStageID)
					m_renderables.maxStageID = stageID;

				// Order stages
				uint2 stageSlot = stageSlots[stageID];

				if (stageSlot < minStageSlot)
				{
					minStageSlot = stageSlot;
					m_renderables.firstStageID = stageID;
				}
				// NOTE: Inclusive as initialized 0
				if (stageSlot >= maxStageSlot)
				{
					maxStageSlot = stageSlot;
					m_renderables.lastStageID = stageID;
				}
			}
		}
	}
}

// Clears all pipeline stages and render queues.
void PipelinePerspective::Dequeue()
{
	// Call custom rendering finalizers
	for (Renderables::perspective_data_vector::iterator it = m_renderables.perspectiveData.begin(); it != m_renderables.perspectiveData.end(); ++it)
	{
		RenderablePerspectiveData &data = *it;
		data.renderable->Finalize(*this, data.pPerspectiveData);
	}
	m_renderables.perspectiveData.clear();

	// Clear render queues
	for (size_t stageID = m_renderables.minStageID; stageID <= m_renderables.maxStageID; ++stageID)
	{
		Stage &stage = m_renderables.stages[stageID];

		for (size_t queueID = stage.minQueueID; queueID <= stage.maxQueueID; ++queueID)
		{
			Queue &queue = stage.queues[queueID];
			queue.sortedJobs.clear();
			queue.renderJobs.clear();
		}

		stage.minQueueID = MaxRenderQueueCount;
		stage.maxQueueID = 0;
		stage.minQueueSlot = MaxRenderQueueCount;
		stage.maxQueueSlot = 0;
	}

	m_renderables.minStageID = MaxPipelineStageCount;
	m_renderables.maxStageID = 0;
	m_renderables.firstStageID = InvalidPipelineStage;
	m_renderables.lastStageID = InvalidPipelineStage;

	// Clear lighting
	m_lights.clear();

	// Free any perspective data
	FreeData();
}

// Prepares rendering of all stages and queues.
void PipelinePerspective::Prepare()
{
	for (uint2 stageID = m_renderables.minStageID; stageID <= m_renderables.maxStageID; ++stageID)
		Prepare(stageID);
}

// Prepares rendering of the given stage.
void PipelinePerspective::Prepare(uint2 stageID)
{
	LEAN_ASSERT(stageID <= MaxPipelineStageCount);

	const Stage &stage = m_renderables.stages[stageID];

	for (uint2 queueID = stage.minQueueID; queueID <= stage.maxQueueID; ++queueID)
		Prepare(stageID, queueID);
}

// Prepares rendering of the given stage and render queue.
void PipelinePerspective::Prepare(uint2 stageID, uint2 queueID)
{
	LEAN_ASSERT(stageID <= MaxPipelineStageCount);
	LEAN_ASSERT(queueID <= MaxRenderQueueCount);

	Queue &queue = m_renderables.stages[stageID].queues[queueID];
	const RenderQueueDesc &queueDesc = m_pPipeline->GetQueueDesc(queueID);

	// Only perform depth sort for queues that request it
	if (queueDesc.DepthSort)
	{
		const uint4 jobCount = static_cast<uint4>( queue.renderJobs.size() );
		queue.sortedJobs.resize(jobCount);

		// Initialize with unsorted jobs
		for (uint4 i = 0; i < jobCount; ++i)
			queue.sortedJobs[i] = i;

		struct DepthOrder
		{
			const QueuedRenderJob *renderJobs;

			explicit DepthOrder(const QueuedRenderJob *renderJobs)
				: renderJobs(renderJobs) { }

			/// Checks if depth of job at left index less than at right index.
			LEAN_INLINE bool operator() (uint4 leftIdx, uint4 rightIdx) const
			{
				return renderJobs[leftIdx].depth < renderJobs[rightIdx].depth;
			}
		};

		// Sort render jobs by their depth
		std::sort(&queue.sortedJobs.front(), &queue.sortedJobs.back() + 1, DepthOrder(&queue.renderJobs.front()));
	}
}

// Optimizes rendering of all stages and queues.
void PipelinePerspective::Optimize()
{
	for (uint2 stageID = m_renderables.minStageID; stageID <= m_renderables.maxStageID; ++stageID)
		Optimize(stageID);
}

// Prepares rendering of the given stage.
void PipelinePerspective::Optimize(uint2 stageID)
{
	LEAN_ASSERT(stageID <= MaxPipelineStageCount);

	const Stage &stage = m_renderables.stages[stageID];

	for (uint2 queueID = stage.minQueueID; queueID <= stage.maxQueueID; ++queueID)
		Optimize(stageID, queueID);
}

// Prepares rendering of the given stage and render queue.
void PipelinePerspective::Optimize(uint2 stageID, uint2 queueID)
{
	LEAN_ASSERT(stageID <= MaxPipelineStageCount);
	LEAN_ASSERT(queueID <= MaxRenderQueueCount);

	Queue &queue = m_renderables.stages[stageID].queues[queueID];
	const RenderQueueDesc &queueDesc = m_pPipeline->GetQueueDesc(queueID);

	// Do not destroy depth sorting
	if (!queueDesc.DepthSort)
	{
		const uint4 jobCount = static_cast<uint4>( queue.renderJobs.size() );
		queue.sortedJobs.resize(jobCount);

		// Initialize with unsorted jobs
		for (uint4 i = 0; i < jobCount; ++i)
			queue.sortedJobs[i] = i;

		struct AnyOrder
		{
			const QueuedRenderJob *renderJobs;

			explicit AnyOrder(const QueuedRenderJob *renderJobs)
				: renderJobs(renderJobs) { }

			/// Checks if sorting index of job at left index less than at right index.
			LEAN_INLINE bool operator() (uint4 leftIdx, uint4 rightIdx) const
			{
				return renderJobs[leftIdx].sortIndex < renderJobs[rightIdx].sortIndex;
			}
		};

		// Sort render jobs by their sorting indices
		std::sort(&queue.sortedJobs.front(), &queue.sortedJobs.back() + 1, AnyOrder(&queue.renderJobs.front()));
	}
}

// Renders the given stage.
void PipelinePerspective::Render(uint2 stageID, const RenderContext &context) const
{
	LEAN_ASSERT(stageID <= MaxPipelineStageCount);

	const Stage &stage = m_renderables.stages[stageID];
	const uint2 *queueIDs = m_pPipeline->GetOrderedQueueIDs();

	for (size_t queueSlot = stage.minQueueSlot; queueSlot <= stage.maxQueueSlot; ++queueSlot)
		Render(stageID, queueIDs[queueSlot], context);
}

// Renders the given stage & render queue.
void PipelinePerspective::Render(uint2 stageID, uint2 queueID, const RenderContext &context) const
{
	LEAN_ASSERT(stageID <= MaxPipelineStageCount);
	LEAN_ASSERT(queueID <= MaxRenderQueueCount);

	const Stage &stage = m_renderables.stages[stageID];
	const Queue &queue = stage.queues[queueID];

	// Stage setup
	if (queueID == stage.firstQueueID)
	{
		const PipelineStageDesc &stageDesc = m_pPipeline->GetStageDesc(stageID);

		if (stageDesc.Setup)
			stageDesc.Setup->SetupRendering(stageID, InvalidRenderQueue, *this, context);
	}

	// Only setup queues that actually rendered something
	if (!queue.renderJobs.empty())
	{
		const RenderQueueDesc &queueDesc = m_pPipeline->GetQueueDesc(queueID);

		// Queue setup
		if (queueDesc.Setup)
			queueDesc.Setup->SetupRendering(stageID, queueID, *this, context);

		// Draw all renderables
		for (Queue::render_job_vector::const_iterator it = queue.renderJobs.begin();
			it != queue.renderJobs.end(); ++it)
		{
			const QueuedRenderJob &renderJob = *it;
			(*renderJob.data.Render)(renderJob.data, *this, nullptr, nullptr, context);
		}
	}

	if (m_pProcessor)
	{
		// Only process queues that actually rendered something
		if (!queue.renderJobs.empty())
		{
			// Generic queue post-processing
			m_pProcessor->Render(InvalidPipelineStage, queueID, this, context);

			// Queue post-processing
			m_pProcessor->Render(stageID, queueID, this, context);
		}

		// Stage post-processing
		if (queueID == stage.lastQueueID)
		{
			m_pProcessor->Render(stageID, InvalidRenderQueue, this, context);

			// Global post-processing
			if (stageID == m_renderables.lastStageID)
				m_pProcessor->Render(InvalidPipelineStage, InvalidRenderQueue, this, context);
		}
	}
}

} // namespace
