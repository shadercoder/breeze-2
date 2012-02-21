/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PROCESSING_EFFECT_DRIVER
#define BE_SCENE_PROCESSING_EFFECT_DRIVER

#include "beScene.h"
#include "beAbstractProcessingEffectDriver.h"
#include "bePipelineEffectBinder.h"
#include "bePerspectiveEffectBinder.h"
#include "bePipeEffectBinder.h"
#include <beGraphics/beEffect.h>
#include <beGraphics/beDeviceContext.h>
#include <beGraphics/beStateManager.h>

namespace beScene
{

/// Processing effect binder.
class ProcessingEffectDriver : public AbstractProcessingEffectDriver
{
protected:
	/// Pipeline effect binder.
	PipelineEffectBinder m_pipelineBinder;
	/// Perspective effect binder.
	PerspectiveEffectBinder m_perspectiveBinder;
	/// Pipe effect binder.
	PipeEffectBinder m_pipeBinder;

public:
	/// Constructor.
	BE_SCENE_API ProcessingEffectDriver(const beGraphics::Technique &technique, RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool);
	/// Destructor.
	BE_SCENE_API ~ProcessingEffectDriver();

	/// Applies the given perspective data to the effect bound by this effect driver.
	BE_SCENE_API bool Apply(const Perspective *pPerspective,
		beGraphics::StateManager& stateManager,const beGraphics::DeviceContext &context) const;

	/// Applies the given pass to the effect bound by this effect driver.
	BE_SCENE_API bool ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const void *pProcessor, const Perspective *pPerspective,
		beGraphics::StateManager& stateManager, const beGraphics::DeviceContext &context) const;

	/// Gets the number of passes.
	BE_SCENE_API uint4 GetPassCount() const;
	/// Gets the pass identified by the given ID.
	BE_SCENE_API const PipelineEffectBinderPass* GetPass(uint4 passID) const;

	/// Gets the pipeline effect binder.
	LEAN_INLINE const PipelineEffectBinder& GetPipelineBinder() const { return m_pipelineBinder; }
	/// Gets the perspective effect binder.
	LEAN_INLINE const PerspectiveEffectBinder& GetPerspectiveBinder() const { return m_perspectiveBinder; }
	/// Gets the pipe effect binder.
	LEAN_INLINE const PipeEffectBinder& GetPipeBinder() const { return m_pipeBinder; }

	/// Gets the effect.
	LEAN_INLINE const beGraphics::Effect& GetEffect() const { return m_perspectiveBinder.GetEffect(); }
};

} // namespace

#endif