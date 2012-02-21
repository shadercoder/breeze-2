/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beSpotLightController.h"
#include "beScene/beMeshGeneration.h"
#include "beScene/beDynamicScenery.h"

#include "beScene/beMeshCache.h"
#include "beScene/beRenderableMaterial.h"

#include <beEntitySystem/beEntity.h>

#include "beScene/bePipePool.h"
#include "beScene/bePerspective.h"
#include "beScene/bePerspectiveScheduler.h"
#include <beGraphics/Any/beTextureTargetPool.h>
#include <beGraphics/Any/beTexture.h>

#include "beScene/beRenderContext.h"
#include <beGraphics/Any/beDevice.h>
#include <beGraphics/Any/beBuffer.h>
#include <beGraphics/Any/beDeviceContext.h>

#include <beCore/beReflectionProperties.h>
#include <lean/smart/weak_resource_ptr.h>

#include <beGraphics/DX/beError.h>

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

namespace beScene
{

const beCore::ReflectionProperties SpotLightControllerProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<float[4]>("color", beCore::Widget::Color)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&SpotLightController::SetColor, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&SpotLightController::GetColor, float) )
	<< beCore::MakeReflectionProperty<float>("attenuation", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::SetAttenuation) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::GetAttenuation) )
	<< beCore::MakeReflectionProperty<float>("attenuation offset", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::SetAttenuationOffset) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::GetAttenuationOffset) )
	<< beCore::MakeReflectionProperty<float>("range", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::SetRange) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::GetRange) )
	<< beCore::MakeReflectionProperty<float>("inner angle", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::SetInnerAngle) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::GetInnerAngle) )
	<< beCore::MakeReflectionProperty<float>("outer angle", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::SetOuterAngle) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::GetOuterAngle) )
	<< beCore::MakeReflectionProperty<bool>("shadow", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::EnableShadow) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::IsShadowEnabled) )
	<< beCore::MakeReflectionProperty<uint4>("shadow resolution", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&SpotLightController::SetShadowResolution) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&SpotLightController::GetShadowResolution) );

namespace
{

/// Creates a spot light constant buffer.
lean::resource_ptr<beGraphics::Any::Buffer, true> CreateSpotLightConstantBuffer(ID3D11Device *pDevice)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(SpotLightConstantBuffer);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = desc.ByteWidth;

	return lean::bind_resource( new beGraphics::Any::Buffer(desc, nullptr, pDevice) );
}

/// Gets the mesh.
const MeshCompound* GetSpotLightMesh(MeshCache &meshCache, const beGraphics::Device &device)
{
	const MeshCompound *pCompound = meshCache.GetMeshByName("beSpotLightController.MeshCompound");

	if (!pCompound)
	{
		lean::resource_ptr<Mesh> pMesh = GenerateCuboidMesh(
			beMath::vec(1.0f, 1.0f, 1.0f),
			beMath::vec(-2.0f, 0.0f, 0.0f),
			beMath::vec(0.0f, -2.0f, 0.0f),
			beMath::vec(0.0f, 0.0f, -1.0f),
			0.0f, 0.0f,
			0.0f, 0.0f,
			1, 1, 1,
			0, device);
		pCompound = meshCache.SetMeshName("beSpotLightController.MeshCompound", pMesh);
	}

	return pCompound;
}

/// Default spot light controller material.
lean::weak_resource_ptr<RenderableMaterial> g_pSpotLightDefaultMaterial;

} // namespace

// Sets the default material for spot lights.
void SetSpotLightDefaultMaterial(RenderableMaterial *pMaterial)
{
	g_pSpotLightDefaultMaterial = pMaterial;
}

// Sets the default material for spot lights.
lean::resource_ptr<RenderableMaterial, true> GetSpotLightDefaultMaterial()
{
	return g_pSpotLightDefaultMaterial.lock().transfer();
}

// Constructor.
SpotLightController::SpotLightController(beEntitySystem::Entity *pEntity, SceneController *pScene,
		DynamicScenery *pScenery, PipePool *pPipePool, MeshCache &meshCache, const beGraphics::Device &device)
	: MeshController(pEntity, pScene, pScenery),
	
	Light(
		GetLightTypes().GetID("SpotLight"),
		CreateSpotLightConstantBuffer(ToImpl(device)).get() ),

	m_pPipePool( LEAN_ASSERT_NOT_NULL(pPipePool) ),

	m_color(1.0f),
	
	m_attenuation(1.0f),
	m_attenuationOffset(1.0f),
	m_range(25.0f),

	m_innerAngle(0.6f),
	m_outerAngle(1.2f),

	m_bConstantsChanged(true),

	m_shadowResolution(1024),
	m_shadowStageMask(2) // TODO: 0 (HACK!)
{
	AddMeshes( *this, *GetSpotLightMesh(meshCache, device), GetSpotLightDefaultMaterial().get() );
}

// Destructor.
SpotLightController::~SpotLightController()
{
}

namespace
{

/// External pass data structure.
struct RenderableLightData
{
	const void *pRenderableData;
	SpotLightController *pController;
};

/// Perspective-dependent light data.
struct PerspectiveLightData
{
	Perspective *pShadowPerspective;
	Pipe *pShadowPipe;
	mutable beGraphics::Any::QualifiedTextureViewHandle pShadowMap;

	/// Constructor.
	PerspectiveLightData(Perspective *pShadowPerspective, Pipe *pShadowPipe, const beGraphics::Any::QualifiedTextureViewHandle &pShadowMap = nullptr)
		: pShadowPerspective(pShadowPerspective),
		pShadowPipe(pShadowPipe),
		pShadowMap(pShadowMap) { }
};

/// Computes the three shadow matrices.
inline void ComputeShadowMatrices(beMath::fmat4 &view, beMath::fmat4 &proj, beMath::fmat4 &viewProj, float &nearPlane, float &farPlane,
	const beMath::fmat3 &orientation, const beMath::fvec3 &position, float angle, float range)
{
	const float Eps = 0.1f;

	view = beMath::mat_view(position, orientation[2], orientation[1], orientation[0]);
	nearPlane = Eps;
	farPlane = nearPlane + range;
	proj = beMath::mat_proj(angle, 1.0f, nearPlane, farPlane);
	viewProj = mul(view, proj);
}

} // namespace

// Prepares this renderable object.
void* SpotLightController::Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const
{
	if (IsShadowEnabled() && m_shadowStageMask)
	{
		const DXGI_SAMPLE_DESC noMS = { 1, 0 };

		const beGraphics::TextureTargetDesc targetDesc(
				m_shadowResolution, m_shadowResolution,
				1,
				beGraphics::Format::R16F,
				beGraphics::SampleDesc()
			);

		const PerspectiveDesc &perspectiveDesc = perspective.GetDesc();

		beMath::fmat4 shadowView, shadowProj, shadowViewProj;
		float shadowMin, shadowMax;
		ComputeShadowMatrices(shadowView, shadowProj, shadowViewProj, shadowMin, shadowMax, 
			m_orientation, m_position, m_outerAngle, m_range);

		PerspectiveDesc shadowPerspectiveDesc(
				m_position,
				m_orientation[0],
				m_orientation[1],
				m_orientation[2],
				shadowView,
				shadowProj,
				shadowMin,
				shadowMax,
				perspectiveDesc.Flipped,
				perspectiveDesc.Time,
				perspectiveDesc.TimeStep
			);

		lean::resource_ptr<Pipe> pShadowPipe = m_pPipePool->GetPipe(targetDesc);

		Perspective *pShadowPerspective = perspectiveScheduler.AddPerspective(shadowPerspectiveDesc, pShadowPipe, nullptr, m_shadowStageMask);

		return new(perspective) PerspectiveLightData(pShadowPerspective, pShadowPipe);
	}
	
	return nullptr;
}

// Gets shadow maps.
const beGraphics::TextureViewHandle* SpotLightController::GetShadowMaps(void *pData, uint4 &count) const
{
	if (pData)
	{
		PerspectiveLightData &data = *static_cast<PerspectiveLightData*>(pData);

		// TODO: check targets, bool bHasShadows...

		if (!data.pShadowMap)
			data.pShadowMap = data.pShadowPipe->GetAnyTarget("SceneShadowTarget")->GetTexture();

		if (data.pShadowMap)
		{
			count = 1;
			return &data.pShadowMap;
		}
	}

	count = 0;
	return nullptr;
}

// Gets light maps.
const beGraphics::TextureViewHandle* SpotLightController::GetLightMaps(void *pData, uint4 &count) const
{
	return Light::GetLightMaps(pData, count);
}

// Called when a renderable is attached to a scenery.
void SpotLightController::Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator)
{
	MeshController::Attached(pScenery, data, allocator);

	// Wrap mesh controller data
	RenderableLightData *pLightData = allocator.AllocateRenderableData<RenderableLightData>();
	pLightData->pRenderableData = data.SharedData;
	pLightData->pController = this;

	// Add light data
	data.Render = &Render;
	data.SharedData = pLightData;
}

// Called for each pass when a renderable is attached to a scenery.
void SpotLightController::Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator)
{
	const RenderableLightData &lightData = *static_cast<const RenderableLightData*>(data.SharedData);
	
	// Unwrap mesh controller data
	RenderableData meshData(data);
	meshData.SharedData = lightData.pRenderableData;

	MeshController::Attached(pScenery, meshData, pass, passIdx, allocator);

	// Enable shadow generation
	pass.Flags |= RenderableFlags::PerspectivePrepare;
}

// Renders this spot light.
void SpotLightController::Render(const RenderJob &job, const Perspective &perspective,
	const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context)
{
	const RenderableLightData &lightData = *static_cast<const RenderableLightData*>(job.SharedData);
	PerspectiveLightData &perspectiveData = *static_cast<PerspectiveLightData*>(job.PerspectiveData);
	
	SpotLightController &self = *lightData.pController;

	if (self.m_bConstantsChanged)
	{
		SpotLightConstantBuffer spotLight;

		memcpy(&spotLight.Right, self.m_orientation[0].cdata(), sizeof(float) * 3);
		spotLight.Right[3] = 0.0f;
		memcpy(&spotLight.Up, self.m_orientation[1].cdata(), sizeof(float) * 3);
		spotLight.Up[3] = 0.0f;
		memcpy(&spotLight.Dir, self.m_orientation[2].cdata(), sizeof(float) * 3);
		spotLight.Dir[3] = 0.0f;
		memcpy(&spotLight.Pos, self.m_position.cdata(), sizeof(float) * 3);
		spotLight.Pos[3] = 1.0f;

		memcpy(&spotLight.Color, self.m_color.cdata(), sizeof(float) * 4);

		spotLight.Attenuation = self.m_attenuation;
		spotLight.AttenuationOffset = self.m_attenuationOffset;
		spotLight.Range = self.m_range;

		spotLight.CosInnerAngle = cos(self.m_innerAngle / 2.0f);
		spotLight.CosOuterAngle = cos(self.m_outerAngle / 2.0f);
		spotLight.SinInnerAngle = sin(self.m_innerAngle / 2.0f);
		spotLight.SinOuterAngle = sin(self.m_outerAngle / 2.0f);

		if (self.IsShadowEnabled())
		{
			spotLight.ShadowResolution[0] = spotLight.ShadowResolution[1] = (float) self.m_shadowResolution;
			spotLight.ShadowPixel[0] = spotLight.ShadowPixel[1] = 1.0f / self.m_shadowResolution;

			memcpy(&spotLight.ShadowProj, perspectiveData.pShadowPerspective->GetDesc().ViewProjMat.cdata(), sizeof(float) * 16);
		}

		ToImpl(context.Context())->UpdateSubresource(
			ToImpl(*self.Light::m_pConstants), 0, nullptr,
			&spotLight, 0, 0);

		self.m_bConstantsChanged = false;
	}

	// Unwrap mesh controller data
	RenderJob meshJob(job);
	meshJob.SharedData = lightData.pRenderableData;

	// Render light
	LightJob thisLight(&self, &perspectiveData);
	MeshController::Render(meshJob, perspective, &thisLight, &thisLight + 1, context);
}

// Synchronizes this controller with the controlled entity.
void SpotLightController::Flush()
{
	m_position = m_pEntity->GetPosition();
	m_orientation = m_pEntity->GetOrientation();

	float radius = m_range * tan(0.5f * m_outerAngle);

	m_pEntity->SetScaling( beMath::vec(radius, radius, m_range) );

	beMath::fsphere3 bounds(
			beMath::nvec<3>(2, 0.5f * m_range),
			sqrt(radius * radius + 0.25f * m_range * m_range)
		);
	SetLocalBounds( beMath::fsphere3( beMath::fvec3(), m_range) );

	m_bConstantsChanged = true;

	MeshController::Flush();
}

SpotLightController::Properties SpotLightController::GetControllerProperties()
{
	return Properties(SpotLightControllerProperties.data(), SpotLightControllerProperties.data_end());
}

// Gets the reflection properties.
SpotLightController::Properties SpotLightController::GetReflectionProperties() const
{
	return Properties(SpotLightControllerProperties.data(), SpotLightControllerProperties.data_end());
}

// Gets the controller type.
utf8_ntr SpotLightController::GetControllerType()
{
	return utf8_ntr("SpotLightController");
}

} // namespace

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/beRenderableMaterialCache.h"

namespace beScene
{

// Creates a spot light.
lean::resource_ptr<SpotLightController, true> CreateSpotLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return lean::new_resource<SpotLightController>(
			pEntity, pScene, pScenery,
			renderer.PipePool(), *resources.MeshCache(), *renderer.Device()
		);
}

namespace
{

/// Default effect.
inline utf8_string& GetDefaultEffectFile()
{
	static utf8_string defaultEffectFile = "Lights/SpotLight.fx";
	return defaultEffectFile;
}

} // namespace

// Sets the default spot light effect file.
void SetSpotLightDefaultEffect(const utf8_ntri &file)
{
	GetDefaultEffectFile().assign(file.begin(), file.end());
}

// Gets the default spot light effect file.
beCore::Exchange::utf8_string GetSpotLightDefaultEffect()
{
	const utf8_string &file = GetDefaultEffectFile();
	return beCore::Exchange::utf8_string(file.begin(), file.end());
}

// Gets the default material for spot lights.
RenderableMaterial* GetSpotLightDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return renderer.RenderableMaterials()->GetMaterial(
			resources.MaterialCache()->GetMaterial(
				resources.EffectCache()->GetEffect(GetDefaultEffectFile(), nullptr, 0),
				"beSpotLightController.Material"
			)
		);
}

// Creates a spot light using the default effect.
lean::resource_ptr<SpotLightController, true> CreateDefaultSpotLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	lean::resource_ptr<SpotLightController> pSpotLight = lean::new_resource<SpotLightController>(
			pEntity, pScene, pScenery,
			renderer.PipePool(), *resources.MeshCache(), *renderer.Device());
	
	pSpotLight->SetMaterial( GetSpotLightDefaultMaterial(resources, renderer) );

	return pSpotLight.transfer();
}

} // namespace
