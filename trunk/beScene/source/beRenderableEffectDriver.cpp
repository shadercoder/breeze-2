/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderableEffectDriver.h"
#include <beGraphics/Any/beEffect.h>
#include <beGraphics/Any/beDeviceContext.h>
#include <beGraphics/Any/beStateManager.h>

namespace beScene
{

namespace
{

} // namespace

// Constructor.
RenderableEffectDriver::RenderableEffectDriver(const beGraphics::Technique &technique, RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool,
											   uint4 flags)
	: m_pipelineBinder( ToImpl(technique), pPipeline, (flags & RenderableEffectDriverFlags::Setup) ? PipelineEffectBinderFlags::AllowUnclassified : 0 ),
	m_renderableBinder( ToImpl(technique), pPool ),
	m_lightBinder( ToImpl(technique) )
{
}

// Destructor.
RenderableEffectDriver::~RenderableEffectDriver()
{
}

// Applies the given renderable & perspective data to the effect bound by this effect driver.
bool RenderableEffectDriver::Apply(const RenderableEffectData *pRenderableData, const Perspective &perspective,
		beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const
{
	return m_renderableBinder.Apply(pRenderableData, perspective, ToImpl(stateManager), ToImpl(context));
}

// Applies the given pass to the effect bound by this effect driver.
bool RenderableEffectDriver::ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const RenderableEffectData *pRenderableData, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd,
		RenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const
{
	const PipelineEffectBinderPass* pPipelinePass = static_cast<const PipelineEffectBinderPass*>(pPass);
	ID3D11DeviceContext *pContextDX = ToImpl(context);
	beGraphics::Any::StateManager &stateManagerDX11 = ToImpl(stateManager);

	uint4 step;
	const StateEffectBinderPass *pStatePass;

	while (step = nextStep++, pStatePass = pPipelinePass->GetPass(step))
	{
		uint4 passID = pStatePass->GetPassID(), nextPassID = passID;

		// Skip invalid light passes
		if (m_lightBinder.Apply(nextPassID, lights, lightsEnd, state.Light, pContextDX))
		{
			// Repeat this step, if suggested by the light effect binder
			if (nextPassID == passID)
				--nextStep;

			return pPipelinePass->Apply(step, stateManagerDX11, pContextDX);
		}
	}

	return false;
}

// Gets the number of passes.
uint4 RenderableEffectDriver::GetPassCount() const
{
	return m_pipelineBinder.GetPassCount();
}

// Gets the pass identified by the given ID.
const PipelineEffectBinderPass* RenderableEffectDriver::GetPass(uint4 passID) const
{
	return m_pipelineBinder.GetPass(passID);
}

} // namespace