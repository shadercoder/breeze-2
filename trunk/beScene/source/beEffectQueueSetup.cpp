/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beEffectQueueSetup.h"
#include "beScene/beAbstractRenderableEffectDriver.h"
#include "beScene/beRenderContext.h"
#include "beScene/beRenderingLimits.h"
#include <beGraphics/Any/beStateManager.h>

namespace beScene
{

// Constructor.
EffectQueueSetup::EffectQueueSetup(AbstractRenderableEffectDriver *pEffectDriver)
	: m_pEffectDriver(pEffectDriver)
{
}

// Destructor.
EffectQueueSetup::~EffectQueueSetup()
{
}

// Called before drawing of a specific render queue begins.
void EffectQueueSetup::SetupRendering(uint4 stageID, uint4 queueID, const Perspective &perspective, const RenderContext &context) const
{
	beGraphics::Any::StateManager &stateManager = ToImpl( context.StateManager() );

	stateManager.Revert();

	AbstractRenderableDriverState driverState;
	m_pEffectDriver->Apply(nullptr, perspective, driverState, stateManager, context.Context());

	const uint4 passCount = m_pEffectDriver->GetPassCount();

	for (uint4 passID = 0; passID < passCount; ++passID)
	{
		const QueuedPass *pPass = m_pEffectDriver->GetPass(passID);
		uint4 passStageID = pPass->GetStageID();
		uint4 passQueueID = pPass->GetQueueID();

		bool bStageMatch = passStageID == stageID || passStageID == InvalidPipelineStage;
		bool bQueueMatch = passQueueID == queueID || passQueueID == InvalidRenderQueue;

		if (bStageMatch && bQueueMatch)
			for (uint4 i = 0;
				m_pEffectDriver->ApplyPass(pPass, i, nullptr, perspective, nullptr, nullptr, driverState, stateManager, context.Context());
				);
	}

	stateManager.RecordOverridden();
}

} // namespace
