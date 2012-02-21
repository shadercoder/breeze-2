/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_ABSTRACT_PROCESSING_EFFECT_DRIVER
#define BE_SCENE_ABSTRACT_PROCESSING_EFFECT_DRIVER

#include "beScene.h"
#include "beEffectDriver.h"
#include "bePassSequence.h"
#include "beQueuedPass.h"
#include <beGraphics/beDeviceContext.h>
#include <beGraphics/beStateManager.h>

namespace beScene
{

// Prototypes.
class Perspective;

/// Processing effect driver base.
class AbstractProcessingEffectDriver : public EffectDriver, public PassSequence<QueuedPass>
{
protected:
	LEAN_INLINE AbstractProcessingEffectDriver& operator =(const AbstractProcessingEffectDriver&) { return *this; }

public:
	/// Applies the given perspective data to the effect bound by this effect driver.
	virtual bool Apply(const Perspective *pPerspective,
		beGraphics::StateManager& stateManager, const beGraphics::DeviceContext &context) const = 0;

	/// Applies the given pass to the effect bound by this effect driver.
	virtual bool ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const void *pProcessor, const Perspective *pPerspective,
		beGraphics::StateManager& stateManager, const beGraphics::DeviceContext &context) const = 0;
};

} // namespace

#endif