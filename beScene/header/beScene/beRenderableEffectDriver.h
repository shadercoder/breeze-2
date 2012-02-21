/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE_EFFECT_DRIVER
#define BE_SCENE_RENDERABLE_EFFECT_DRIVER

#include "beScene.h"
#include "beAbstractRenderableEffectDriver.h"
#include "bePipelineEffectBinder.h"
#include "beRenderableEffectBinder.h"
#include "beLightEffectBinder.h"
#include <beGraphics/beEffect.h>
#include <beGraphics/beDeviceContext.h>
#include <beGraphics/beStateManager.h>

namespace beScene
{

/// Renderable effect driver.
class RenderableEffectDriver : public AbstractRenderableEffectDriver
{
protected:
	PipelineEffectBinder m_pipelineBinder;		///< Pipeline effect binder.
	RenderableEffectBinder m_renderableBinder;	///< Renderable effect binder.
	LightEffectBinder m_lightBinder;			///< Light effect binder.

public:
	/// Constructor.
	BE_SCENE_API RenderableEffectDriver(const beGraphics::Technique &technique, RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool,
		uint4 flags = 0);
	/// Destructor.
	BE_SCENE_API ~RenderableEffectDriver();

	/// Applies the given renderable & perspective data to the effect bound by this effect driver.
	BE_SCENE_API bool Apply(const RenderableEffectData *pRenderableData, const Perspective &perspective,
		beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const;

	/// Applies the given pass to the effect bound by this effect driver.
	BE_SCENE_API bool ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const RenderableEffectData *pRenderableData, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd,
		RenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const;

	/// Gets the number of passes.
	BE_SCENE_API uint4 GetPassCount() const;
	/// Gets the pass identified by the given ID.
	BE_SCENE_API const PipelineEffectBinderPass* GetPass(uint4 passID) const;

	/// Gets the pipeline effect binder.
	LEAN_INLINE const PipelineEffectBinder& GetPipelineBinder() const { return m_pipelineBinder; }
	/// Gets the renderable effect binder.
	LEAN_INLINE const RenderableEffectBinder& GetRenderableBinder() const { return m_renderableBinder; }

	/// Gets the effect.
	LEAN_INLINE const beGraphics::Effect& GetEffect() const { return m_pipelineBinder.GetEffect(); }
};

} // namespace

#endif