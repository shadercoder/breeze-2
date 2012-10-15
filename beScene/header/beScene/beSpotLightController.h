/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_SPOT_LIGHT_CONTROLLER
#define BE_SCENE_SPOT_LIGHT_CONTROLLER

#include "beScene.h"
#include "beMeshController.h"
#include "beLight.h"
#include "beRenderingLimits.h"
#include <beEntitySystem/beEntity.h>

#include <beGraphics/beDevice.h>

#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>

namespace beScene
{

/// Spot light constant buffer layout.
struct SpotLightConstantBuffer
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

	float4 CosInnerAngle;		///< Light inner cone angle.
	float4 CosOuterAngle;		///< Light outer cone angle.
	float4 SinInnerAngle;		///< Light inner cone angle.
	float4 SinOuterAngle;		///< Light outer cone angle.

	float4 ShadowResolution[2];	///< Shadow resolution.
	float4 ShadowPixel[2];		///< Shadow pixel (= 1 / resolution).

	float4 ShadowProj[4][4];	///< Shadow projection matrix.
};

// Prototypes
class PipePool;
class MeshCache;

/// Spot light controller.
class SpotLightController : public MeshController, public Light
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

	/// Inner angle.
	float m_innerAngle;
	/// Outer angle.
	float m_outerAngle;

	/// Constants changed flag.
	mutable bool m_bConstantsChanged;

	/// Shadow stage mask.
	PipelineStageMask m_shadowStageMask;

	/// Shadow resolution.
	uint4 m_shadowResolution;

public:
	/// Constructor.
	BE_SCENE_API SpotLightController(beEntitySystem::Entity *pEntity,  SceneController *pScene,
		DynamicScenery *pScenery, PipePool *pPipePool, MeshCache &meshCache, const beGraphics::Device &device);
	/// Destructor.
	BE_SCENE_API ~SpotLightController();

	/// Prepares this renderable object.
	BE_SCENE_API void* Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const;

	/// Called when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator);
	/// Called for each pass when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator);

	/// Renders this spot light.
	BE_SCENE_API static void Render(const RenderJob &job, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context);

	/// Synchronizes this controller with the controlled entity.
	BE_SCENE_API void Flush();

	/// Gets shadow maps.
	BE_SCENE_API const beGraphics::TextureViewHandle* GetShadowMaps(void *pData, uint4 &count) const;
	/// Gets light maps.
	BE_SCENE_API const beGraphics::TextureViewHandle* GetLightMaps(void *pData, uint4 &count) const;

	/// Sets the color.
	LEAN_INLINE void SetColor(const beMath::fvec4 &color) { m_color = color; m_bConstantsChanged = true; EmitPropertyChanged(); }
	/// Gets the color.
	LEAN_INLINE const beMath::fvec4& GetColor() const { return m_color; }

	/// Sets the attenuation.
	LEAN_INLINE void SetAttenuation(float attenuation) { m_attenuation = attenuation; m_bConstantsChanged = true; EmitPropertyChanged(); }
	/// Sets the attenuation offset.
	LEAN_INLINE void SetAttenuationOffset(float attenuationOffset) { m_attenuationOffset = attenuationOffset; m_bConstantsChanged = true; EmitPropertyChanged(); }
	/// Gets the attenuation offset.
	LEAN_INLINE float GetAttenuationOffset() const { return m_attenuationOffset; }
	/// Gets the attenuation.
	LEAN_INLINE float GetAttenuation() const { return m_attenuation; }
	
	/// Sets the range.
	LEAN_INLINE void SetRange(float range) { m_range = range; m_bConstantsChanged = true; EmitPropertyChanged(); }
	/// Gets the range.
	LEAN_INLINE float GetRange() const { return m_range; }

	/// Sets the angles.
	LEAN_INLINE void SetInnerAngle(float innerAngle) { m_innerAngle = innerAngle; m_bConstantsChanged = true; EmitPropertyChanged(); }
	/// Sets the angles.
	LEAN_INLINE void SetOuterAngle(float outerAngle) { m_outerAngle = outerAngle; m_bConstantsChanged = true; EmitPropertyChanged(); }
	/// Gets the inner angle.
	LEAN_INLINE float GetInnerAngle() const { return m_innerAngle; }
	/// Gets the outer angle.
	LEAN_INLINE float GetOuterAngle() const { return m_outerAngle; }

	/// Enables shadow casting.
	LEAN_INLINE void EnableShadow(bool bEnable)
	{
		if (bEnable)
			Light::m_flags |= LightFlags::Shadowed;
		else
			Light::m_flags &= ~LightFlags::Shadowed;

		EmitPropertyChanged();
	}
	/// Checks if this light is currently casting shadows.
	LEAN_INLINE bool IsShadowEnabled() const { return ((Light::m_flags & LightFlags::Shadowed) != 0); }

	/// Sets the shadow stage mask.
	LEAN_INLINE void SetShadowStages(PipelineStageMask shadowStages) { m_shadowStageMask = shadowStages; }
	/// Gets the shadow stage mask.
	LEAN_INLINE PipelineStageMask GetShadowStages() const { return m_shadowStageMask; }

	/// Sets the shadow resolution.
	LEAN_INLINE void SetShadowResolution(uint4 resolution) { m_shadowResolution = resolution; m_bConstantsChanged = true; EmitPropertyChanged(); }
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

/// Sets the default material for spot lights.
BE_SCENE_API void SetSpotLightDefaultMaterial(RenderableMaterial *pMaterial);
/// Gets the default material for spot lights.
BE_SCENE_API lean::resource_ptr<RenderableMaterial, true> GetSpotLightDefaultMaterial();

class ResourceManager;
class EffectDrivenRenderer;

/// Creates a spot light.
BE_SCENE_API lean::resource_ptr<SpotLightController, true> CreateSpotLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

/// Sets the default spot light effect file.
BE_SCENE_API void SetSpotLightDefaultEffect(const utf8_ntri &file);
/// Gets the default spot light effect file.
BE_SCENE_API beCore::Exchange::utf8_string GetSpotLightDefaultEffect();

/// Gets the default material for spot lights.
BE_SCENE_API RenderableMaterial* GetSpotLightDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer);
/// Creates a spot light using the default effect.
BE_SCENE_API lean::resource_ptr<SpotLightController, true> CreateDefaultSpotLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

} // namespace

#endif