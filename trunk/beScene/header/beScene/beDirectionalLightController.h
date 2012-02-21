/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_DIRECTIONAL_LIGHT_CONTROLLER
#define BE_SCENE_DIRECTIONAL_LIGHT_CONTROLLER

#include "beScene.h"
#include "beMeshController.h"
#include "beRenderingLimits.h"
#include "beLight.h"

#include <beGraphics/beDevice.h>

#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>

namespace beScene
{

/// Directional light constant buffer layout.
struct DirectionalLightConstantBuffer
{
	float4 Right[4];			///< Light right.
	float4 Up[4];				///< Light up.
	float4 Dir[4];				///< Light direction.
	float4 Pos[4];				///< Light position.

	float4 Color[4];			///< Light color.

	float4 Attenuation;			///< Light attenuation.
	float4 AttenuationOffset;	///< Light attenuation offset.
	float4 Range;				///< Light range.
	float4 AttPadding;

	float4 ShadowResolution[2];	///< Shadow resolution.
	float4 ShadowPixel[2];		///< Shadow pixel (= 1 / resolution).

	static const uint4 MaxShadowSplitCount = 4;		///< Maximum shadow split count.

	float4 ShadowSplitPlanes[MaxShadowSplitCount];	///< Split plane distances

	/// Shadow split.
	struct ShadowSplit
	{
		float4 Proj[4][4];		///< Shadow projection matrix.
		float4 Near;			///< Near plane.
		float4 Far;				///< Far plane.
		float4 PixelScaleX;		///< 1 / pixel width.
		float4 PixelScaleY;		///< 1 / pixel height.
	};

	ShadowSplit ShadowSplits[MaxShadowSplitCount];	///< Shadow spits.
};

// Prototypes
class PipePool;
class MeshCache;

/// Directional light controller.
class DirectionalLightController : public MeshController, public Light
{
protected:
	/// Pipe pool
	lean::resource_ptr<PipePool> m_pPipePool;

	/// Orientation.
	beMath::fmat3 m_orientation;
	/// Position.
	beMath::fvec3 m_position;

	/// Color.
	beMath::fvec4 m_color;

	/// Attenuation.
	float m_attenuation;
	/// Attenuation offset.
	float m_attenuationOffset;
	/// Range.
	float m_range;

	/// Constants changed flag.
	mutable bool m_bConstantsChanged;

	/// Shadow stage mask.
	PipelineStageMask m_shadowStageMask;

	/// Shadow resolution.
	uint4 m_shadowResolution;

public:
	/// Constructor.
	BE_SCENE_API DirectionalLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
		PipePool *pPipePool, MeshCache &meshCache, const beGraphics::Device &device);
	/// Destructor.
	BE_SCENE_API ~DirectionalLightController();

	/// Prepares this renderable object.
	BE_SCENE_API void* Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const;

	/// Called when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator);
	/// Called for each pass when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator);

	/// Renders this directional light.
	BE_SCENE_API static void Render(const RenderJob &job, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context);

	/// Synchronizes this controller with the controlled entity.
	BE_SCENE_API void Flush();

	/// Gets shadow maps.
	BE_SCENE_API const beGraphics::TextureViewHandle* GetShadowMaps(const void *pData, uint4 &count) const;
	/// Gets light maps.
	BE_SCENE_API const beGraphics::TextureViewHandle* GetLightMaps(const void *pData, uint4 &count) const;

	/// Sets the color.
	LEAN_INLINE void SetColor(const beMath::fvec4 &color) { m_color = color; m_bConstantsChanged = true; }
	/// Gets the color.
	LEAN_INLINE const beMath::fvec4& GetColor() const { return m_color; }

	/// Sets the attenuation.
	LEAN_INLINE void SetAttenuation(float attenuation) { m_attenuation = attenuation; m_bConstantsChanged = true; }
	/// Sets the attenuation offset.
	LEAN_INLINE void SetAttenuationOffset(float attenuationOffset) { m_attenuationOffset = attenuationOffset; m_bConstantsChanged = true; }
	/// Gets the attenuation offset.
	LEAN_INLINE float GetAttenuationOffset() const { return m_attenuationOffset; }
	/// Gets the attenuation.
	LEAN_INLINE float GetAttenuation() const { return m_attenuation; }
	
	/// Sets the range.
	LEAN_INLINE void SetRange(float range) { m_range = range; m_bConstantsChanged = true; }
	/// Gets the range.
	LEAN_INLINE float GetRange() const { return m_range; }

	/// Enables shadow casting.
	LEAN_INLINE void EnableShadow(bool bEnable)
	{
		if (bEnable)
			Light::m_flags |= LightFlags::Shadowed;
		else
			Light::m_flags &= ~LightFlags::Shadowed;
	}
	/// Checks if this light is currently casting shadows.
	LEAN_INLINE bool IsShadowEnabled() const { return ((Light::m_flags & LightFlags::Shadowed) != 0); }

	/// Sets the shadow stage mask.
	LEAN_INLINE void SetShadowStages(PipelineStageMask shadowStages) { m_shadowStageMask = shadowStages; }
	/// Gets the shadow stage mask.
	LEAN_INLINE PipelineStageMask GetShadowStages() const { return m_shadowStageMask; }

	/// Sets the shadow resolution.
	LEAN_INLINE void SetShadowResolution(uint4 resolution) { m_shadowResolution = resolution; m_bConstantsChanged = true; }
	/// Gets the shadow resolution.
	LEAN_INLINE uint4 GetShadowResolution() const { return m_shadowResolution; }

	/// Gets the reflection properties.
	BE_SCENE_API static Properties GetControllerProperties();
	/// Gets the reflection properties.
	BE_SCENE_API Properties GetReflectionProperties() const;

	/// Gets the controller type.
	BE_SCENE_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

/// Sets the default material for directional lights.
void SetDirectionalLightDefaultMaterial(RenderableMaterial *pMaterial);
/// Gets the default material for directional lights.
lean::resource_ptr<RenderableMaterial, true> GetDirectionalLightDefaultMaterial();

class ResourceManager;
class EffectDrivenRenderer;

/// Creates a directional light.
lean::resource_ptr<DirectionalLightController, true> CreateDirectionalLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

/// Sets the default directional light effect file.
void SetDirectionalLightDefaultEffect(const utf8_ntri &file);
/// Gets the default directional light effect file.
beCore::Exchange::utf8_string GetDirectionalLightDefaultEffect();

/// Gets the default material for directional lights.
RenderableMaterial* GetDirectionalLightDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer);
/// Creates a directional light using the default effect.
lean::resource_ptr<DirectionalLightController, true> CreateDefaultDirectionalLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

} // namespace

#endif