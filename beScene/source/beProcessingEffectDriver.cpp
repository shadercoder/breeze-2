/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beProcessingEffectDriver.h"
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
ProcessingEffectDriver::ProcessingEffectDriver(const beGraphics::Technique &technique, RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool)
	: m_pipelineBinder( ToImpl(technique), pPipeline, PipelineEffectBinderFlags::AllowUnclassified ),
	m_perspectiveBinder( ToImpl(technique), pPool ),
	m_pipeBinder( ToImpl(technique), PipeEffectBinderFlags::NoDefaultMS )
{
}

// Destructor.
ProcessingEffectDriver::~ProcessingEffectDriver()
{
}

// Applies the given perspective data to the effect bound by this effect driver.
bool ProcessingEffectDriver::Apply(const Perspective *pPerspective, beGraphics::StateManager& stateManager, const beGraphics::DeviceContext &context) const
{
	return (pPerspective)
		? m_perspectiveBinder.Apply(*pPerspective, ToImpl(stateManager), ToImpl(context))
		: true;
}

// Applies the given pass to the effect bound by this effect driver.
bool ProcessingEffectDriver::ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
	const void *pProcessor, const Perspective *pPerspective,
	beGraphics::StateManager& stateManager, const beGraphics::DeviceContext &context) const
{
	const PipelineEffectBinderPass* pPipelinePass = static_cast<const PipelineEffectBinderPass*>(pPass);
	ID3D11DeviceContext *pContextDX = ToImpl(context);
	beGraphics::Any::StateManager &stateManagerDX11 = ToImpl(stateManager);

	uint4 step = nextStep++;
	const StateEffectBinderPass *pStatePass = pPipelinePass->GetPass(step);

	if (pStatePass)
	{
		uint4 passID = pStatePass->GetPassID();
		uint4 nextPassID = passID;

		DX11::Pipe *pPipe = nullptr;
		uint4 outIndex = 0;

		if (pPerspective)
		{
			pPipe = ToImpl(pPerspective->GetPipe());
			outIndex = pPerspective->GetDesc().OutputIndex;
		}

		if (m_pipeBinder.Apply(nextPassID, pPipe, outIndex, pProcessor, stateManagerDX11, pContextDX))
		{
			if (nextPassID == passID)
				// Repeat this step, if suggested by the pipe effect binder
				--nextStep;

			return pStatePass->Apply(stateManagerDX11, pContextDX);
		}
	}

	return false;
}

// Gets the number of passes.
uint4 ProcessingEffectDriver::GetPassCount() const
{
	return m_pipelineBinder.GetPassCount();
}

// Gets the pass identified by the given ID.
const PipelineEffectBinderPass* ProcessingEffectDriver::GetPass(uint4 passID) const
{
	return m_pipelineBinder.GetPass(passID);
}

} // namespace