/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE_PROCESSING_EFFECT_DRIVER
#define BE_SCENE_RENDERABLE_PROCESSING_EFFECT_DRIVER

#include "beScene.h"
#include "beRenderableEffectDriver.h"
#include "bePipeEffectBinder.h"
#include <beGraphics/beEffect.h>
#include <beGraphics/beDeviceContext.h>
#include <beGraphics/beStateManager.h>

namespace beScene
{

/// Renderable effect binder.
class RenderableProcessingEffectDriver : public RenderableEffectDriver
{
protected:
	PipeEffectBinder m_pipeBinder;		///< Pipe effect binder.

public:
	/// Constructor.
	BE_SCENE_API RenderableProcessingEffectDriver(const beGraphics::Technique &technique, RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool,
		uint4 flags = 0);
	/// Destructor.
	BE_SCENE_API ~RenderableProcessingEffectDriver();

	/// Applies the given pass to the effect bound by this effect driver.
	BE_SCENE_API bool ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const RenderableEffectData *pRenderableData, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd,
		AbstractRenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const;

	/// Gets the pipe effect binder.
	LEAN_INLINE const PipeEffectBinder& GetPipeBinder() const { return m_pipeBinder; }
};

} // namespace

#endif