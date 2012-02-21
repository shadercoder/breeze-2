/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERINGLIMITS
#define BE_SCENE_RENDERINGLIMITS

#include "beScene.h"
#include <bitset>

#ifndef BE_SCENE_MAX_RENDER_QUEUE_COUNT

	/// Maximum number of pipeline stages any renderable may be added to.
	/// @ingroup GlobalSwitches
	/// @ingroup SceneLibrary
	#define BE_SCENE_MAX_PIPELINE_STAGE_COUNT 32

	/// Maximum number of render queues any renderable may be added to.
	/// @ingroup GlobalSwitches
	/// @ingroup SceneLibrary
	#define BE_SCENE_MAX_RENDER_QUEUE_COUNT 32

#endif

namespace beScene
{

/// Maximum number of pipeline stages any renderable may be added to.
const uint2 MaxPipelineStageCount = BE_SCENE_MAX_PIPELINE_STAGE_COUNT;
/// Maximum number of pipeline stages any renderable may be added to.
const uint2 MaxRenderQueueCount = BE_SCENE_MAX_RENDER_QUEUE_COUNT;

/// Stage ID type.
typedef uint2 PipelineStageID;
/// Queue ID type.
typedef uint2 RenderQueueID;

/// Invalid stage ID.
const PipelineStageID InvalidPipelineStage = static_cast<PipelineStageID>(-1);
/// Invalid queue ID.
const RenderQueueID InvalidRenderQueue = static_cast<RenderQueueID>(-1);

/// Stage mask type.
typedef uint4 PipelineStageMask;
/// Queue mask type.
typedef uint4 RenderQueueMask;

LEAN_STATIC_ASSERT(lean::size_info<PipelineStageMask>::bits >= MaxPipelineStageCount);
LEAN_STATIC_ASSERT(lean::size_info<RenderQueueMask>::bits >= MaxRenderQueueCount);

} // namespace

#endif