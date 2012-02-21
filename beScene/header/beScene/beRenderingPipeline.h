/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERINGPIPELINE
#define BE_SCENE_RENDERINGPIPELINE

#include "beScene.h"
#include <beCore/beShared.h>
#include <lean/pimpl/pimpl_ptr.h>
#include "beRenderingLimits.h"
#include "bePerspectiveScheduler.h"
#include "bePipelinePerspective.h"
#include "beQueueSetup.h"
#include <lean/smart/resource_ptr.h>

namespace beScene
{
	
// Prototypes
class Scenery;
class PerspectiveModifier;
class Pipe;
class RenderContext;
class PipelineProcessor;

/// Pipeline stage description.
struct PipelineStageDesc
{
	int4 Layer;		///< Layer index.
	bool Normal;	///< Part of normal rendering.
	lean::resource_ptr<const QueueSetup> Setup;		///< Queue setup handler.

	/// Constructor.
	LEAN_INLINE explicit PipelineStageDesc(
		int4 layer,
		bool bNormal = true,
		const QueueSetup *pSetup = nullptr)
			: Layer(layer),
			Normal(bNormal),
			Setup(pSetup)  { }
};

/// Render queue description.
struct RenderQueueDesc
{
	int4 Layer;										///< Layer index.
	bool DepthSort;									///< Depth sort flag.
	lean::resource_ptr<const QueueSetup> Setup;		///< Queue setup handler.

	/// Constructor.
	LEAN_INLINE explicit RenderQueueDesc(
		int4 layer,
		bool bDepthSort = false,
		const QueueSetup *pSetup = nullptr)
			: Layer(layer),
			DepthSort(bDepthSort),
			Setup(pSetup) { }
};

/// Rendering Pipeline.
class RenderingPipeline : public beCore::Resource, public PerspectiveScheduler
{
private:
	utf8_string m_name;

	class Impl;
	lean::pimpl_ptr<Impl> m_impl;

public:
	/// Constructor.
	BE_SCENE_API RenderingPipeline(const utf8_ntri &name);
	/// Destructor.
	BE_SCENE_API ~RenderingPipeline();

	/// Adds a perspective.
	BE_SCENE_API PipelinePerspective* AddPerspective(const PerspectiveDesc &desc, Pipe *pPipe,
		PipelineProcessor *pProcessor = nullptr, PipelineStageMask stageMask = 0);
	/// Clears all perspectives.
	BE_SCENE_API void ClearPerspectives();

	/// Releases shared references held.
	BE_SCENE_API void Release();

	/// Sets a perspective modifier (nullptr to unset), returning any previous perspective modifier.
	BE_SCENE_API PerspectiveModifier* SetPerspectiveModifier(PerspectiveModifier *pModifier);

	/// Includes all visible objects in the given scenery.
	BE_SCENE_API void AddScenery(const Scenery &scenery);
	
	/// Prepares rendering of all stages and queues.
	BE_SCENE_API void Prepare();
	/// Prepares rendering of the given stage.
	BE_SCENE_API void Prepare(uint2 stageID);

	/// Renders all stages and queues.
	BE_SCENE_API void Render(const RenderContext &context) const;
	/// Renders the given stage and render queue.
	BE_SCENE_API void Render(uint2 stageID, const RenderContext &context) const;

	/// Renders all stages and queues including the given scenery.
	BE_SCENE_API void Render(const Scenery &scenery, const RenderContext &context);

	/// Adds a pipeline stage according to the given description.
	BE_SCENE_API uint2 AddStage(const utf8_ntri &stageName, const PipelineStageDesc &desc);
	/// Adds a render queue according to the given description.
	BE_SCENE_API uint2 AddQueue(const utf8_ntri &queueName, const RenderQueueDesc &desc);

	/// Sets a new setup for the given pipeline stage.
	BE_SCENE_API void SetStageSetup(uint2 stageID, const QueueSetup *pSetup);
	/// Sets a new setup for the given render queue.
	BE_SCENE_API void SetQueueSetup(uint2 queueID, const QueueSetup *pSetup);

	/// Gets the ID of the pipeline stage identified by the given name.
	BE_SCENE_API uint2 GetStageID(const utf8_ntri &stageName) const;
	/// Gets the ID of the render queue identified by the given name.
	BE_SCENE_API uint2 GetQueueID(const utf8_ntri &queueName) const;

	/// Gets the number of pipeline stages.
	BE_SCENE_API uint2 GetStageCount() const;
	/// Gets the number of render queues.
	BE_SCENE_API uint2 GetQueueCount() const;

	/// Gets the description of the given pipeline stage.
	BE_SCENE_API const PipelineStageDesc& GetStageDesc(uint2 stageID) const;
	/// Gets the description of the given pipeline stage.
	BE_SCENE_API const RenderQueueDesc& GetQueueDesc(uint2 queueID) const;

	/// Gets an ordered sequence of pipeline stage IDs.
	BE_SCENE_API const uint2* GetOrderedStageIDs() const;
	/// Gets a pipeline stage slot lookup table.
	BE_SCENE_API const uint2* GetStageSlots() const;
	/// Gets an ordered sequence of render queue IDs.
	BE_SCENE_API const uint2* GetOrderedQueueIDs() const;
	/// Gets a render queue slot lookup table.
	BE_SCENE_API const uint2* GetQueueSlots() const;

	/// Gets the ID of the default pipeline stage.
	BE_SCENE_API void SetDefaultStageID(uint2 stageID);
	/// Gets the ID of the default pipeline stage.
	BE_SCENE_API uint2 GetDefaultStageID() const;
	/// Gets the ID of the default pipeline stage.
	BE_SCENE_API void SetDefaultQueueID(uint2 stageID, uint2 queueID);
	/// Gets the ID of the default pipeline stage.
	BE_SCENE_API uint2 GetDefaultQueueID(uint2 stageID) const;

	/// Sets the maximum number of active perspectives.
	BE_SCENE_API void SetMaxPerspectiveCount(uint4 count);
	/// Gets the maximum number of active perspectives.
	BE_SCENE_API uint4 GetMaxPerspectiveCount() const;

	/// Sets the name.
	BE_SCENE_API void SetName(const utf8_ntri &name);
	/// Gets the name.
	LEAN_INLINE const utf8_string& GetName() const { return m_name; }
};

} // nmaespace

#endif