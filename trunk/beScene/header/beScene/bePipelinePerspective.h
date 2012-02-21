/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PIPELINE_PERSPECTIVE
#define BE_SCENE_PIPELINE_PERSPECTIVE

#include "beScene.h"
#include "bePerspective.h"
#include "beRenderingLimits.h"
#include <vector>
#include <lean/containers/simple_vector.h>
#include <lean/containers/accumulation_vector.h>
#include "beRenderJob.h"
#include "beRenderable.h"
#include <lean/smart/resource_ptr.h>

namespace beScene
{
	
// Prototypes
class Scenery;
class Renderable;
class Light;
struct LightJob;
class Pipe;
class RenderingPipeline;
class RenderContext;
class PipelineProcessor;

/// Rendering perspective.
class PipelinePerspective : public Perspective
{
public:
	/// Queued render job.
	struct QueuedRenderJob
	{
		uint4 sortIndex;				///< Sort index.
		uint4 depth;					///< Camera depth.
		RenderJob data;					///< Render job.
	};

	/// Render queue.
	struct Queue
	{ 
		typedef lean::simple_vector<QueuedRenderJob, lean::simple_vector_policies::pod> render_job_vector;
		render_job_vector renderJobs;

		typedef lean::simple_vector<uint4> sorted_job_vector;
		sorted_job_vector sortedJobs;
	};

	/// Pipeline stage.
	struct Stage
	{
		typedef Queue queue_vector[MaxRenderQueueCount];
		queue_vector queues;
		uint2 minQueueID;
		uint2 maxQueueID;
		uint2 minQueueSlot;
		uint2 maxQueueSlot;
		uint2 firstQueueID;
		uint2 lastQueueID;

		/// Constructor.
		LEAN_INLINE Stage()
			: minQueueID(MaxRenderQueueCount),
			maxQueueID(0),
			minQueueSlot(MaxRenderQueueCount),
			maxQueueSlot(0),
			firstQueueID(InvalidRenderQueue),
			lastQueueID(InvalidRenderQueue) { }
	};

	/// Perspective data.
	struct RenderablePerspectiveData
	{
		const Renderable *renderable;
		void *pPerspectiveData;

		/// NON-INITIALIZING constructor.
		RenderablePerspectiveData() { }
		/// Constructor.
		RenderablePerspectiveData(const Renderable *renderable, void *pPerspectiveData)
			: renderable(renderable),
			pPerspectiveData(pPerspectiveData) { }
	};

	/// Renderables.
	struct Renderables
	{
		typedef Stage stage_vector[MaxPipelineStageCount];
		stage_vector stages;
		uint2 minStageID;
		uint2 maxStageID;
		uint2 firstStageID;
		uint2 lastStageID;

		PipelineStageMask stageMask;

		typedef lean::simple_vector<RenderablePerspectiveData> perspective_data_vector;
		perspective_data_vector perspectiveData;

		/// Constructor.
		LEAN_INLINE Renderables(PipelineStageMask stageMask)
			: minStageID(MaxPipelineStageCount),
			maxStageID(0),
			firstStageID(InvalidPipelineStage),
			lastStageID(InvalidPipelineStage),
			stageMask(stageMask) { }
	};

	typedef lean::simple_vector<const Light*, lean::simple_vector_policies::pod> light_vector;

private:
	RenderingPipeline *m_pPipeline;

	Renderables m_renderables;
	light_vector m_lights;

	Pipe *m_pPipe;

	PipelineProcessor *m_pProcessor;

public:
	/// Constructor.
	BE_SCENE_API PipelinePerspective(RenderingPipeline *pPipeline, const PerspectiveDesc &desc, Pipe *pPipe, PipelineProcessor *pProcessor, PipelineStageMask stageMask);
	/// Destructor.
	BE_SCENE_API ~PipelinePerspective();

	/// Sets the perspective description and resets all contents.
	BE_SCENE_API void Reset(const PerspectiveDesc &desc, Pipe *pPipe, PipelineProcessor *pProcessor, PipelineStageMask stageMask);
	/// Releases shared references held.
	BE_SCENE_API void Release();

	/// Adds all visible renderables in the given scenery to this perspective.
	BE_SCENE_API void AddRenderables(const Scenery& scenery);
	/// Adds the given renderable to this perspective.
	BE_SCENE_API void AddRenderables(const Renderable *pRenderable, const RenderableData &renderableData);

	/// Adds all visible lights in the given scenery to this perspective.
	BE_SCENE_API void AddLights(const Scenery& scenery);
	/// Adds the given light to this perspective.
	BE_SCENE_API void AddLight(const Light *pLight);

	/// Queues all renderables into their render queues and pipeline stages
	BE_SCENE_API void Enqueue();
	/// Clears all pipeline stages and render queues.
	BE_SCENE_API void Dequeue();

	/// Prepares rendering of all stages and queues.
	BE_SCENE_API void Prepare();
	/// Prepares rendering of the given stage.
	BE_SCENE_API void Prepare(uint2 stageID);
	/// Prepares rendering of the given stage and render queue.
	BE_SCENE_API void Prepare(uint2 stageID, uint2 queueID);

	/// Optimizes rendering of all stages and queues.
	BE_SCENE_API void Optimize();
	/// Prepares rendering of the given stage.
	BE_SCENE_API void Optimize(uint2 stageID);
	/// Prepares rendering of the given stage and render queue.
	BE_SCENE_API void Optimize(uint2 stageID, uint2 queueID);

	/// Renders the given stage.
	BE_SCENE_API void Render(uint2 stageID, const RenderContext &context) const;
	/// Renders the given stage & render queue.
	BE_SCENE_API void Render(uint2 stageID, uint2 queueID, const RenderContext &context) const;

	/// Optionally gets a pipe.
	LEAN_INLINE Pipe* GetPipe() const { return m_pPipe; }

	/// Gets a stage mask.
	LEAN_INLINE PipelineStageMask GetStageMask() const { return m_renderables.stageMask; }
};

} // namespace

#endif