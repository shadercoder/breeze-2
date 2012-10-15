/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_LIGHT_EFFECT_BINDER
#define BE_SCENE_LIGHT_EFFECT_BINDER

#include "beScene.h"
#include "beEffectBinder.h"
#include <beGraphics/Any/beEffect.h>
#include <beGraphics/Any/beEffectsAPI.h>
#include <lean/smart/resource_ptr.h>
#include <vector>

namespace beScene
{

// Prototypes
struct LightJob;
class RenderingPipeline;

/// Light effect binder state.
struct LightBinderState
{
	uint4 LightOffset;	///< Current light offset in additive lighting.

	/// Constructor.
	LightBinderState()
		: LightOffset(0) { }
};

/// Light effect binder.
class LightEffectBinder : public EffectBinder
{
public:
	struct Pass;
	typedef std::vector<Pass> pass_vector;

	struct LightGroup;
	typedef std::vector<LightGroup> light_group_vector;

private:
	const beGraphics::Any::Technique m_technique;

	light_group_vector m_lightGroups;

	beGraphics::Any::API::EffectScalar *m_pLightCount;

	pass_vector m_passes;

public:
	/// Constructor.
	BE_SCENE_API LightEffectBinder(const beGraphics::Any::Technique &technique, uint4 passID = static_cast<uint4>(-1));
	/// Destructor.
	BE_SCENE_API ~LightEffectBinder();

	/// Applies the n-th step of the given pass.
	BE_SCENE_API bool Apply(uint4 &nextPassID, const LightJob *lights, const LightJob *lightsEnd,
		LightBinderState &lightState, beGraphics::Any::API::DeviceContext *pContext) const;

	/// Gets the technique.
	LEAN_INLINE const beGraphics::Technique& GetTechnique() const { return m_technique; }
	/// Gets the effect.
	LEAN_INLINE const beGraphics::Effect& GetEffect() const { return *m_technique.GetEffect(); }
};

} // namespace

#endif