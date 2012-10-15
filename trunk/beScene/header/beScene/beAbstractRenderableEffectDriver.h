/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_ABSTRACT_RENDERABLE_EFFECT_DRIVER
#define BE_SCENE_ABSTRACT_RENDERABLE_EFFECT_DRIVER

#include "beScene.h"
#include "beEffectDriver.h"
#include "bePassSequence.h"
#include "beQueuedPass.h"
#include "beRenderableEffectData.h"
#include <beGraphics/beDeviceContext.h>
#include <beGraphics/beStateManager.h>

namespace beScene
{

// Prototypes.
class Perspective;
class Renderable;
struct LightJob;

/// Renderable effect driver state.
struct AbstractRenderableDriverState
{
	char Data[4 * sizeof(uint4)];
};

/// Renderable effect driver flags enumeration.
namespace RenderableEffectDriverFlags
{
	/// Enumeration.
	enum T
	{
		Setup = 0x1		///< Treats effect as setup effect.
	};
}

/// Renderable effect driver base.
class AbstractRenderableEffectDriver : public EffectDriver, public PassSequence<QueuedPass>
{
protected:
	LEAN_INLINE AbstractRenderableEffectDriver& operator =(const AbstractRenderableEffectDriver&) { return *this; }

public:
	/// Applies the given renderable & perspective data to the effect bound by this effect driver.
	virtual bool Apply(const RenderableEffectData *pRenderableData, const Perspective &perspective,
		AbstractRenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const = 0;

	/// Applies the given pass to the effect bound by this effect driver.
	virtual bool ApplyPass(const QueuedPass *pPass, uint4 &nextStep,
		const RenderableEffectData *pRenderableData, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd,
		AbstractRenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const = 0;

	/// Draws the given number of primitives.
	virtual void DrawIndexed(uint4 indexCount, uint4 startIndex, int4 baseVertex,
		AbstractRenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const = 0;
	/// Draws the given number of primitives and instances.
	virtual void DrawIndexedInstanced(uint4 indexCount, uint4 instanceCount, uint4 startIndex, uint4 startInstance, int4 baseVertex,
		AbstractRenderableDriverState &state, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context) const = 0;
};

} // namespace

#endif