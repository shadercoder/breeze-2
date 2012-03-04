/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/bePointLightController.h"
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

const beCore::ReflectionProperties PointLightControllerProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<float[4]>("color", beCore::Widget::Color)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&PointLightController::SetColor, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&PointLightController::GetColor, float) )
	<< beCore::MakeReflectionProperty<float>("attenuation", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&PointLightController::SetAttenuation) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&PointLightController::GetAttenuation) )
	<< beCore::MakeReflectionProperty<float>("attenuation offset", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&PointLightController::SetAttenuationOffset) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&PointLightController::GetAttenuationOffset) )
	<< beCore::MakeReflectionProperty<float>("range", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&PointLightController::SetRange) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&PointLightController::GetRange) )
	<< beCore::MakeReflectionProperty<bool>("shadow", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&PointLightController::EnableShadow) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&PointLightController::IsShadowEnabled) )
	<< beCore::MakeReflectionProperty<uint4>("shadow resolution", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&PointLightController::SetShadowResolution) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&PointLightController::GetShadowResolution) );

namespace
{

/// Creates a point light constant buffer.
lean::resource_ptr<beGraphics::Any::Buffer, true> CreatePointLightConstantBuffer(ID3D11Device *pDevice)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(PointLightConstantBuffer);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = desc.ByteWidth;

	return lean::bind_resource( new beGraphics::Any::Buffer(desc, nullptr, pDevice) );
}

/// Gets the mesh.
const MeshCompound* GetPointLightMesh(MeshCache &meshCache, const beGraphics::Device &device)
{
	const MeshCompound *pCompound = meshCache.GetMeshByName("bePointLightController.MeshCompound");

	if (!pCompound)
	{
		lean::resource_ptr<Mesh> pMesh = GenerateCuboidMesh(
			beMath::vec(1.0f, 1.0f, 1.0f),
			beMath::vec(-2.0f, 0.0f, 0.0f),
			beMath::vec(0.0f, -2.0f, 0.0f),
			beMath::vec(0.0f, 0.0f, -2.0f),
			0.0f, 0.0f,
			0.0f, 0.0f,
			1, 1, 1,
			0, device);
		pCompound = meshCache.SetMeshName("bePointLightController.MeshCompound", pMesh);
	}

	return pCompound;
}

/// Default point light controller material.
lean::weak_resource_ptr<RenderableMaterial> g_pPointLightDefaultMaterial;

} // namespace

// Sets the default material for point lights.
void SetPointLightDefaultMaterial(RenderableMaterial *pMaterial)
{
	g_pPointLightDefaultMaterial = pMaterial;
}

// Sets the default material for point lights.
lean::resource_ptr<RenderableMaterial, true> GetPointLightDefaultMaterial()
{
	return g_pPointLightDefaultMaterial.lock().transfer();
}

// Constructor.
PointLightController::PointLightController(beEntitySystem::Entity *pEntity,  SceneController *pScene,
		DynamicScenery *pScenery, PipePool *pPipePool, MeshCache &meshCache, const beGraphics::Device &device)
	: MeshController(pEntity, pScene, pScenery),

	Light(
		GetLightTypes().GetID("PointLight"),
		CreatePointLightConstantBuffer(ToImpl(device)).get() ),

	m_pPipePool( LEAN_ASSERT_NOT_NULL(pPipePool) ),

	m_color(1.0f),
	
	m_attenuation(1.0f),
	m_attenuationOffset(1.0f),
	m_range(25.0f),

	m_bConstantsChanged(true),

	m_shadowResolution(512),
	m_shadowStageMask(2) // TODO: 0 (HACK!)
{
	AddMeshes( *this, *GetPointLightMesh(meshCache, device), GetPointLightDefaultMaterial().get() );
}

// Destructor.
PointLightController::~PointLightController()
{
}

namespace
{

/// External pass data structure.
struct RenderableLightData
{
	const void *pRenderableData;
	PointLightController *pController;
};

/// Perspective-dependent light data.
struct PerspectiveLightData
{
	Perspective *pShadowPerspectives[PointLightConstantBuffer::ShadowFaceCount];
	Pipe *pShadowPipes[PointLightConstantBuffer::ShadowFaceCount];
	mutable beGraphics::Any::QualifiedTextureViewHandle pShadowMaps[PointLightConstantBuffer::ShadowFaceCount];

	/// Constructor.
	PerspectiveLightData()
		: pShadowPerspectives(),
		pShadowPipes(),
		pShadowMaps() { }
};

/// Computes the shadow orientation.
inline beMath::fmat3 ComputeShadowOrientation(const beMath::fmat3 &orientation, uint4 index)
{
	float viewSign = (index < 3) ? 1.0f : -1.0f;

	return beMath::mat_transform(
		orientation[(2 + index) % 2] * viewSign,
		orientation[(1 + index) % 2],
		orientation[(0 + index) % 2] * viewSign);
}

/// Computes the three shadow matrices.
inline void ComputeShadowMatrices(beMath::fmat4 &view, beMath::fmat4 &proj, beMath::fmat4 &viewProj, float &nearPlane, float &farPlane, 
	const beMath::fmat3 &orientation, const beMath::fvec3 &position, float range)
{
	const float Eps = 0.1f;

	view = beMath::mat_view(position, orientation[2], orientation[1], orientation[0]);
	nearPlane = Eps;
	farPlane = nearPlane + range;
	proj = beMath::mat_proj(beMath::pi<float>::quarter, 1.0f, nearPlane, farPlane);
	viewProj = mul(view, proj);
}

} // namespace

// Prepares this renderable object.
void* PointLightController::Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const
{
	if (IsShadowEnabled() && m_shadowStageMask)
	{
		PerspectiveLightData lightData;

		const beGraphics::TextureTargetDesc targetDesc(
				m_shadowResolution, m_shadowResolution,
				1,
				beGraphics::Format::R16F,
				beGraphics::SampleDesc()
			);

		const PerspectiveDesc &perspectiveDesc = perspective.GetDesc();

		for (uint4 i = 0; i < PointLightConstantBuffer::ShadowFaceCount; ++i)
		{
			beMath::fmat3 shadowOrientation = ComputeShadowOrientation(m_orientation, i);

			beMath::fmat4 shadowView, shadowProj, shadowViewProj;
			float shadowMin, shadowMax;
			ComputeShadowMatrices(shadowView, shadowProj, shadowViewProj, shadowMin, shadowMax, 
				shadowOrientation, m_position, m_range);

			PerspectiveDesc shadowPerspectiveDesc(
					m_position,
					shadowOrientation[0],
					shadowOrientation[1],
					shadowOrientation[2],
					shadowView,
					shadowProj,
					shadowMin,
					shadowMax,
					perspectiveDesc.Flipped,
					perspectiveDesc.Time,
					perspectiveDesc.TimeStep
				);

			lean::resource_ptr<Pipe> pShadowPipe = m_pPipePool->GetPipe(targetDesc);

			lightData.pShadowPipes[i] = pShadowPipe;
			lightData.pShadowPerspectives[i] = perspectiveScheduler.AddPerspective(shadowPerspectiveDesc, pShadowPipe, nullptr, m_shadowStageMask);
		}

		return new(perspective) PerspectiveLightData(lightData);
	}
	
	return nullptr;
}

// Gets shadow maps.
const beGraphics::TextureViewHandle* PointLightController::GetShadowMaps(const void *pData, uint4 &count) const
{
	if (pData)
	{
		const PerspectiveLightData &lightData = *static_cast<const PerspectiveLightData*>(pData);

		// TODO: check targets, bool bHasShadows...

		if (!lightData.pShadowMaps[0])
			for (uint4 i = 0; i < PointLightConstantBuffer::ShadowFaceCount; ++i)
				lightData.pShadowMaps[i] = lightData.pShadowPipes[i]->GetAnyTarget("SceneShadowTarget")->GetTexture();

		if (lightData.pShadowMaps[0])
		{
			count = PointLightConstantBuffer::ShadowFaceCount;
			return lightData.pShadowMaps;
		}
	}

	count = 0;
	return nullptr;
}

// Gets light maps.
const beGraphics::TextureViewHandle* PointLightController::GetLightMaps(const void *pData, uint4 &count) const
{
	return Light::GetLightMaps(pData, count);
}

// Called when a renderable is attached to a scenery.
void PointLightController::Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator)
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
void PointLightController::Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator)
{
	const RenderableLightData &lightData = *static_cast<const RenderableLightData*>(data.SharedData);
	
	// Unwrap mesh controller data
	RenderableData meshData(data);
	meshData.SharedData = lightData.pRenderableData;

	MeshController::Attached(pScenery, meshData, pass, passIdx, allocator);

	// Enable shadow generation
	pass.Flags |= RenderableFlags::PerspectivePrepare;
}

// Renders this point light.
void PointLightController::Render(const RenderJob &job, const Perspective &perspective,
	const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context)
{
	const RenderableLightData &lightData = *static_cast<const RenderableLightData*>(job.SharedData);
	PerspectiveLightData &perspectiveData = *static_cast<PerspectiveLightData*>(job.PerspectiveData);

	PointLightController &self = *lightData.pController;

	if (self.m_bConstantsChanged)
	{
		PointLightConstantBuffer pointLight;

		memcpy(&pointLight.Right, self.m_orientation[0].cdata(), sizeof(float) * 3);
		pointLight.Right[3] = 0.0f;
		memcpy(&pointLight.Up, self.m_orientation[1].cdata(), sizeof(float) * 3);
		pointLight.Up[3] = 0.0f;
		memcpy(&pointLight.Dir, self.m_orientation[2].cdata(), sizeof(float) * 3);
		pointLight.Dir[3] = 0.0f;
		memcpy(&pointLight.Pos, self.m_position.cdata(), sizeof(float) * 3);
		pointLight.Pos[3] = 1.0f;

		memcpy(&pointLight.Color, self.m_color.cdata(), sizeof(float) * 4);

		pointLight.Attenuation = self.m_attenuation;
		pointLight.AttenuationOffset = self.m_attenuationOffset;
		pointLight.Range = self.m_range;

		if (self.IsShadowEnabled())
		{
			pointLight.ShadowResolution[0] = pointLight.ShadowResolution[1] = (float) self.m_shadowResolution;
			pointLight.ShadowPixel[0] = pointLight.ShadowPixel[1] = 1.0f / self.m_shadowResolution;

			for (uint4 i = 0; i < PointLightConstantBuffer::ShadowFaceCount; ++i)
				memcpy(pointLight.ShadowFaces[i].Proj,
					perspectiveData.pShadowPerspectives[i]->GetDesc().ViewProjMat.cdata(),
					sizeof(float) * 16);
		}

		ToImpl(context.Context())->UpdateSubresource(
			ToImpl(*self.Light::m_pConstants), 0, nullptr,
			&pointLight, 0, 0);

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
void PointLightController::Flush()
{
	m_position = m_pEntity->GetPosition();
	m_orientation = m_pEntity->GetOrientation();

	m_pEntity->SetScaling( beMath::vec(m_range, m_range, m_range) );

	m_bConstantsChanged = true;

	MeshController::Flush();
}

// Gets the reflection properties.
PointLightController::Properties PointLightController::GetControllerProperties()
{
	return Properties(PointLightControllerProperties.data(), PointLightControllerProperties.data_end());
}

// Gets the reflection properties.
PointLightController::Properties PointLightController::GetReflectionProperties() const
{
	return Properties(PointLightControllerProperties.data(), PointLightControllerProperties.data_end());
}

// Gets the controller type.
utf8_ntr PointLightController::GetControllerType()
{
	return utf8_ntr("PointLightController");
}

} // namespace

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/beRenderableMaterialCache.h"

namespace beScene
{

// Creates a point light.
lean::resource_ptr<PointLightController, true> CreatePointLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return lean::new_resource<PointLightController>(
			pEntity, pScene, pScenery,
			renderer.PipePool(), *resources.MeshCache(), *renderer.Device()
		);
}

namespace
{

/// Default effect.
inline utf8_string& GetDefaultEffectFile()
{
	static utf8_string defaultEffectFile = "Lights/PointLight.fx";
	return defaultEffectFile;
}

} // namespace

// Sets the default point light effect file.
void SetPointLightDefaultEffect(const utf8_ntri &file)
{
	GetDefaultEffectFile().assign(file.begin(), file.end());
}

// Gets the default point light effect file.
beCore::Exchange::utf8_string GetPointLightDefaultEffect()
{
	const utf8_string &file = GetDefaultEffectFile();
	return beCore::Exchange::utf8_string(file.begin(), file.end());
}

// Gets the default material for point lights.
RenderableMaterial* GetPointLightDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return renderer.RenderableMaterials()->GetMaterial(
			resources.MaterialCache()->GetMaterial(
				resources.EffectCache()->GetEffect(GetDefaultEffectFile(), nullptr, 0),
				"bePointLightController.Material"
			)
		);
}

// Creates a point light using the default effect.
lean::resource_ptr<PointLightController, true> CreateDefaultPointLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	lean::resource_ptr<PointLightController> pPointLight = lean::new_resource<PointLightController>(
			pEntity, pScene, pScenery,
			renderer.PipePool(), *resources.MeshCache(), *renderer.Device());

	pPointLight->SetMaterial( GetPointLightDefaultMaterial(resources, renderer) );

	return pPointLight.transfer();
}

} // namespace
