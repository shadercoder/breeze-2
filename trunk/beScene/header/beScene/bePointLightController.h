/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_POINT_LIGHT_CONTROLLER
#define BE_SCENE_POINT_LIGHT_CONTROLLER

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

/// Point light constant buffer layout.
struct PointLightConstantBuffer
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

	/// Shadow face.
	struct ShadowFace
	{
		float4 Proj[4][4];		///< Shadow projection matrix.
	};
	static const uint4 ShadowFaceCount = 6;		///< Number of shadow faces.

	ShadowFace ShadowFaces[ShadowFaceCount];	///< Shadow faces.
};

// Prototypes
class PipePool;
class MeshCache;

/// Point light controller.
class PointLightController : public MeshController, public Light
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
	BE_SCENE_API PointLightController(beEntitySystem::Entity *pEntity,  SceneController *pScene,
		DynamicScenery *pScenery, PipePool *pPipePool, MeshCache &meshCache, const beGraphics::Device &device);
	/// Destructor.
	BE_SCENE_API ~PointLightController();

	/// Prepares this renderable object.
	BE_SCENE_API void* Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const;

	/// Called when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator);
	/// Called for each pass when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator);

	/// Renders this point light.
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

/// Sets the default material for point lights.
void SetPointLightDefaultMaterial(RenderableMaterial *pMaterial);
/// Gets the default material for point lights.
lean::resource_ptr<RenderableMaterial, true> GetPointLightDefaultMaterial();

class ResourceManager;
class EffectDrivenRenderer;

/// Creates a point light.
lean::resource_ptr<PointLightController, true> CreatePointLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

/// Sets the default point light effect file.
void SetPointLightDefaultEffect(const utf8_ntri &file);
/// Gets the default point light effect file.
beCore::Exchange::utf8_string GetPointLightDefaultEffect();

/// Gets the default material for point lights.
RenderableMaterial* GetPointLightDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer);
/// Creates a point light using the default effect.
lean::resource_ptr<PointLightController, true> CreateDefaultPointLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

} // namespace

#endif