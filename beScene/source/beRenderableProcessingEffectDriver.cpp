/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderableProcessingEffectDriver.h"
#include "beScene/bePerspective.h"
#include "beScene/DX11/bePipe.h"
#include <beGraphics/Any/beEffect.h>
#include <beGraphics/Any/beDeviceContext.h>
#include <beGraphics/Any/beStateManager.h>

namespace beScene
{

namespace
{

} // namespace

// Constructor.
RenderableProcessingEffectDriver::RenderableProcessingEffectDriver(const beGraphics::Technique &technique, RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool,
																   uint4 flags)
	: RenderableEffectDriver( technique, pPipeline, pPool, flags ),
	m_pipeBinder( ToImpl(technique) )
{
}

// Destructor.
RenderableProcessingEffectDriver::~RenderableProcessingEffectDriver()
{
}

// Applies the given pass to the effect bound by this effect driver.
bool RenderableProcessingEffectDriver::ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const RenderableEffectData *pRenderableData, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd,
		AbstractRenderableDriverState &abstractState, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const
{
	const PipelineEffectBinderPass* pPipelinePass = static_cast<const PipelineEffectBinderPass*>(pPass);
	RenderableDriverState &state = ToRenderableDriverState<RenderableDriverState>(abstractState);
	beGraphics::Any::StateManager &stateManagerDX11 = ToImpl(stateManager);
	ID3D11DeviceContext *pContextDX = ToImpl(context);

	uint4 step;
	const StateEffectBinderPass *pStatePass;

	while (step = nextStep++, pStatePass = pPipelinePass->GetPass(step))
	{
		uint4 passID = pStatePass->GetPassID(), nextPassID = passID;

		// Skip invalid light passes
		if (m_lightBinder.Apply(nextPassID, lights, lightsEnd, state.Light, pContextDX))
		{
			// Repeat this step, if suggested by the light effect binder
			bool bRepeat = (nextPassID == passID);

			// NOTE: Reset for pipe effect binder
			nextPassID = passID;

			if (m_pipeBinder.Apply(nextPassID, ToImpl(perspective.GetPipe()), perspective.GetDesc().OutputIndex, pRenderableData, stateManagerDX11, pContextDX))
				// Repeat this step, if suggested by the pipe effect binder
				bRepeat |= (nextPassID == passID);
			
			if (bRepeat)
				--nextStep;

			state.PassID = passID;
			return pPipelinePass->Apply(step, stateManagerDX11, pContextDX);
		}
	}

	return false;
}

} // namespace