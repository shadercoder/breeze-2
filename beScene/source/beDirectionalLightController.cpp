/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beDirectionalLightController.h"
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
#include <beMath/beProjection.h>

namespace beScene
{

const beCore::ReflectionProperties DirectionalLightControllerProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<float[4]>("color", beCore::Widget::Color)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&DirectionalLightController::SetColor, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&DirectionalLightController::GetColor, float) )
	<< beCore::MakeReflectionProperty<float[4]>("sky color", beCore::Widget::Color)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&DirectionalLightController::SetSkyColor, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&DirectionalLightController::GetSkyColor, float) )
	<< beCore::MakeReflectionProperty<float>("attenuation", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&DirectionalLightController::SetAttenuation) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&DirectionalLightController::GetAttenuation) )
	<< beCore::MakeReflectionProperty<float>("attenuation offset", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&DirectionalLightController::SetAttenuationOffset) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&DirectionalLightController::GetAttenuationOffset) )
	<< beCore::MakeReflectionProperty<float>("range", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&DirectionalLightController::SetRange) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&DirectionalLightController::GetRange) )
	<< beCore::MakeReflectionProperty<bool>("shadow", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&DirectionalLightController::EnableShadow) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&DirectionalLightController::IsShadowEnabled) )
	<< beCore::MakeReflectionProperty<uint4>("shadow resolution", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&DirectionalLightController::SetShadowResolution) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&DirectionalLightController::GetShadowResolution) );

namespace
{

/// Creates a directional light constant buffer.
lean::resource_ptr<beGraphics::Any::Buffer, true> CreateDirectionalLightConstantBuffer(ID3D11Device *pDevice)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(DirectionalLightConstantBuffer);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = desc.ByteWidth;

	return lean::bind_resource( new beGraphics::Any::Buffer(desc, nullptr, pDevice) );
}

/// Gets the mesh.
const MeshCompound* GetDirectionalLightMesh(MeshCache &meshCache, const beGraphics::Device &device)
{
	const MeshCompound *pCompound = meshCache.GetMeshByName("beDirectionalLightController.MeshCompound");

	if (!pCompound)
	{
		lean::resource_ptr<Mesh> pMesh = GenerateGridMesh(
			beMath::vec(-1.0f, -1.0f, 1.0f),
			beMath::vec(2.0f, 0.0f, 0.0f),
			beMath::vec(0.0f, 2.0f, 0.0f),
			0.0f, 0.0f, 0.0f, 0.0f,
			1, 1,
			0, device);
		pCompound = meshCache.SetMeshName("beDirectionalLightController.MeshCompound", pMesh);
	}

	return pCompound;
}

/// Default directional light controller material.
lean::weak_resource_ptr<RenderableMaterial> g_pDirectionalLightDefaultMaterial;

} // namespace

// Sets the default material for directional lights.
void SetDirectionalLightDefaultMaterial(RenderableMaterial *pMaterial)
{
	g_pDirectionalLightDefaultMaterial = pMaterial;
}

// Sets the default material for directional lights.
lean::resource_ptr<RenderableMaterial, true> GetDirectionalLightDefaultMaterial()
{
	return g_pDirectionalLightDefaultMaterial.lock().transfer();
}

// Constructor.
DirectionalLightController::DirectionalLightController(beEntitySystem::Entity *pEntity, SceneController *pScene,
		DynamicScenery *pScenery, PipePool *pPipePool, MeshCache &meshCache, const beGraphics::Device &device)
	: MeshController(pEntity, pScene, pScenery),
	Light(
			GetLightTypes().GetID("DirectionalLight"),
			CreateDirectionalLightConstantBuffer(ToImpl(device)).get()
		),

	m_pPipePool( LEAN_ASSERT_NOT_NULL(pPipePool) ),

	m_color(1.0f),
	m_sky(1.0f),
	
	m_attenuation(1.0f),
	m_attenuationOffset(1.0f),
	m_range(512.0f),

	m_bConstantsChanged(true),

	m_shadowResolution(1024),
	m_shadowStageMask(2) // TODO: 0 (HACK!)
{
	AddMeshes( *this, *GetDirectionalLightMesh(meshCache, device), GetDirectionalLightDefaultMaterial().get() );
}

// Destructor.
DirectionalLightController::~DirectionalLightController()
{
}

namespace
{

/// External pass data structure.
struct RenderableLightData
{
	const void *pRenderableData;
	DirectionalLightController *pController;
};

/// Perspective-dependent light data.
struct PerspectiveLightData
{
	Perspective *pShadowPerspectives[DirectionalLightConstantBuffer::MaxShadowSplitCount];
	float splitPlanes[DirectionalLightConstantBuffer::MaxShadowSplitCount];
	beMath::fvec2 pixelScale[DirectionalLightConstantBuffer::MaxShadowSplitCount];
	Pipe *pShadowPipe;
	mutable beGraphics::Any::QualifiedTextureViewHandle pShadowMap;

	/// Constructor.
	PerspectiveLightData()
		: pShadowPerspectives(),
		pShadowPipe(),
		pShadowMap() { }
};

/// Computes the shadow orientation.
inline float ComputeSplitOffset(float nearPlane, float farPlane, uint4 index)
{
	float percentage = (float) index / DirectionalLightConstantBuffer::MaxShadowSplitCount;

	return 0.5f * (
			nearPlane + (farPlane - nearPlane) * percentage
			+ nearPlane * pow(farPlane / nearPlane, percentage)
		);
}

/// Computes the three shadow matrices.
inline void ComputeShadowMatrices(beMath::fmat4 &view, beMath::fmat4 &proj, beMath::fmat4 &viewProj, beMath::fvec2 &pixelScale,
	beMath::fvec3 &center, float &nearPlane, float &farPlane,
	const beMath::fmat3 &camOrientation, const beMath::fvec3 &camPosition, const beMath::fmat4 &camViewProj,
	float splitNear, float splitFar,
	const beMath::fmat3 &orientation, float range, uint4 resolution,
	bool bOmnidirectional)
{
	center = camPosition;
	if (!bOmnidirectional)
		center += 0.5f * (splitNear + splitFar) * camOrientation[2];

	view = beMath::mat_view(center, orientation[2], orientation[1], orientation[0]);

	beMath::fvec3 cornerPoints[8];

	if (!bOmnidirectional)
	{
		beMath::fvec3 centers[2];
		centers[0] = camPosition + splitNear * camOrientation[2];
		centers[1] = camPosition + splitFar * camOrientation[2];

		beMath::fplane3 sides[2];
		sides[0] = frustum_left(camViewProj);
		sides[1] = frustum_right(camViewProj);

		beMath::fvec3 sideCenters[4];

		for (int i = 0; i < 2; ++i)
		{
			// Project side normal to camera plane
			beMath::fvec3 toSideDir = sides[i].n() - camOrientation[2] * dot(sides[i].n(), camOrientation[2]);
			// Scale to erase side plane distance
			toSideDir /= -dot(toSideDir, sides[i].n());
		
			for (int j = 0; j < 2; ++j)
				// Project split centers to side
				sideCenters[2 * j + i] = centers[j] + toSideDir * sdist(sides[i], centers[j]);
		}

		beMath::fplane3 vertSides[2];
		vertSides[0] = frustum_top(camViewProj);
		vertSides[1] = frustum_bottom(camViewProj);

		for (int i = 0; i < 2; ++i)
		{
			// Compute vector orthogonal to both side & cam plane
			beMath::fvec3 orthoSide = cross(sides[i].n(), camOrientation[2]);
		
			for (int j = 0; j < 2; ++j)
			{
				// Scale to erase vertical side plane distance
				beMath::fvec3 toVertSide = orthoSide / -dot(orthoSide, vertSides[j].n());

				for (int k = 0; k < 2; ++k)
				{
					const beMath::fvec3 &sideCenter = sideCenters[2 * k + i];

					// Project side center to vertical side
					cornerPoints[4 * k + 2 * j + i] = sideCenter + toVertSide * sdist(vertSides[j], sideCenter);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					cornerPoints[4 * k + 2 * j + i] = camPosition
						+ (i ? -splitFar : splitFar) * camOrientation[2]
						+ (j ? -splitFar : splitFar) * camOrientation[1]
						+ (k ? -splitFar : splitFar) * camOrientation[0];
	}

	beMath::fvec3 splitSpaceMin(2.0e16f), splitSpaceMax(-2.0e16f);

	for (int i = 0; i < 8; ++i)
	{
		// Transform split corner points to split space to compute bounds
		beMath::fvec3 splitSpaceCorner = mulh(cornerPoints[i], view);
		splitSpaceMin = min_cw(splitSpaceCorner, splitSpaceMin);
		splitSpaceMax = max_cw(splitSpaceCorner, splitSpaceMax);
	}

	// Snap resolution changes
	float pixelWidth = (splitSpaceMax[0] - splitSpaceMin[0]) / resolution;
	float pixelHeight = (splitSpaceMax[1] - splitSpaceMin[1]) / resolution;

	static const float Log2 = log(2.0f);

	float snappedPixelWidth = exp( Log2 * ceil( log(pixelWidth) / Log2 ) );
	float snappedPixelHeight = exp( Log2 * ceil( log(pixelHeight) / Log2 ) );

	float snappedRangeX = snappedPixelWidth * resolution;
	float snappedRangeY = snappedPixelHeight * resolution;

	// Snap view
	center -= orientation[0] * fmod( dot(center, orientation[0]), snappedPixelWidth );
	center -= orientation[1] * fmod( dot(center, orientation[1]), snappedPixelHeight );

	view = beMath::mat_view(center, orientation[2], orientation[1], orientation[0]);
	
	// Snap projection
	splitSpaceMin[0] = floor(splitSpaceMin[0] / snappedPixelWidth - 1.0f) * snappedPixelWidth;
	splitSpaceMin[1] = floor(splitSpaceMin[1] / snappedPixelHeight - 1.0f) * snappedPixelHeight;
	// Need EXACTLY the snapped resolution
	splitSpaceMax[0] = splitSpaceMin[0] + snappedRangeX;
	splitSpaceMax[1] = splitSpaceMin[1] + snappedRangeY;

	nearPlane = splitSpaceMin[2] - 32.0f * range;
	farPlane = splitSpaceMax[2];
	proj = beMath::mat_proj_ortho(splitSpaceMin[0], splitSpaceMin[1],
		splitSpaceMax[0], splitSpaceMax[1],
		nearPlane, farPlane);
	
	viewProj = mul(view, proj);

	pixelScale[0] = 1.0f / snappedRangeX;
	pixelScale[1] = 1.0f / snappedRangeY;
}

} // namespace

// Prepares this renderable object.
void* DirectionalLightController::Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const
{
	if (IsShadowEnabled() && m_shadowStageMask)
	{
		PerspectiveLightData lightData;

		const beGraphics::TextureTargetDesc targetDesc(
				m_shadowResolution, m_shadowResolution,
				1,
				beGraphics::Format::R16F,
				beGraphics::SampleDesc(),
				DirectionalLightConstantBuffer::MaxShadowSplitCount
			);

		lean::resource_ptr<Pipe> pSplitPipe = m_pPipePool->GetPipe(targetDesc);

		const PerspectiveDesc &perspectiveDesc = perspective.GetDesc();
		const beMath::fmat3 camOrientation = mat_transform(perspectiveDesc.CamLook, perspectiveDesc.CamUp, perspectiveDesc.CamRight);

		for (uint4 i = 0; i < DirectionalLightConstantBuffer::MaxShadowSplitCount; ++i)
		{
			float splitNear = ComputeSplitOffset(perspectiveDesc.NearPlane, m_range, i);
			float splitFar = ComputeSplitOffset(perspectiveDesc.NearPlane, m_range, i + 1);

			lightData.splitPlanes[i] = splitFar;

			beMath::fmat4 splitView, splitProj, splitViewProj;
			beMath::fvec3 splitCenter;
			float splitMin, splitMax;
			ComputeShadowMatrices(splitView, splitProj, splitViewProj, lightData.pixelScale[i],
				splitCenter, splitMin, splitMax,
				camOrientation, perspectiveDesc.CamPos, perspectiveDesc.ViewProjMat,
				splitNear, splitFar,
				m_orientation, m_range, m_shadowResolution,
				perspectiveDesc.Flags & PerspectiveFlags::Omnidirectional);

			PerspectiveDesc splitPerspectiveDesc(
					splitCenter,
					m_orientation[0],
					m_orientation[1],
					m_orientation[2],
					splitView,
					splitProj,
					splitMin,
					splitMax,
					perspectiveDesc.Flipped,
					perspectiveDesc.Time,
					perspectiveDesc.TimeStep,
					i
				);

			lightData.pShadowPerspectives[i] = perspectiveScheduler.AddPerspective(splitPerspectiveDesc, pSplitPipe, nullptr, m_shadowStageMask);
		}

		lightData.pShadowPipe = pSplitPipe;

		return new(perspective) PerspectiveLightData(lightData);
	}
	
	return nullptr;
}

// Gets shadow maps.
const beGraphics::TextureViewHandle* DirectionalLightController::GetShadowMaps(const void *pData, uint4 &count) const
{
	if (pData)
	{
		const PerspectiveLightData &data = *static_cast<const PerspectiveLightData*>(pData);

		if (!data.pShadowMap && data.pShadowPipe)
		{
			const beGraphics::TextureTarget *pTarget = data.pShadowPipe->GetAnyTarget("SceneShadowTarget");

			if (pTarget)
				data.pShadowMap = pTarget->GetTexture();
		}

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
const beGraphics::TextureViewHandle* DirectionalLightController::GetLightMaps(const void *pData, uint4 &count) const
{
	return Light::GetLightMaps(pData, count);
}

// Called when a renderable is attached to a scenery.
void DirectionalLightController::Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator)
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
void DirectionalLightController::Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator)
{
	const RenderableLightData &lightData = *static_cast<const RenderableLightData*>(data.SharedData);
	
	// Unwrap mesh controller data
	RenderableData meshData(data);
	meshData.SharedData = lightData.pRenderableData;

	MeshController::Attached(pScenery, meshData, pass, passIdx, allocator);

	// Enable shadow generation
	pass.Flags |= RenderableFlags::PerspectivePrepare;
}

// Renders this directional light.
void DirectionalLightController::Render(const RenderJob &job, const Perspective &perspective,
	const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context)
{
	const RenderableLightData &lightData = *static_cast<const RenderableLightData*>(job.SharedData);
	PerspectiveLightData &perspectiveData = *static_cast<PerspectiveLightData*>(job.PerspectiveData);

	DirectionalLightController &self = *lightData.pController;

	// WRONG: Perspectives may change!
//	if (self.m_bConstantsChanged)
	{
		DirectionalLightConstantBuffer directionalLight;

		memcpy(&directionalLight.Right, self.m_orientation[0].cdata(), sizeof(float) * 3);
		directionalLight.Right[3] = 0.0f;
		memcpy(&directionalLight.Up, self.m_orientation[1].cdata(), sizeof(float) * 3);
		directionalLight.Up[3] = 0.0f;
		memcpy(&directionalLight.Dir, self.m_orientation[2].cdata(), sizeof(float) * 3);
		directionalLight.Dir[3] = 0.0f;
		memcpy(&directionalLight.Pos, self.m_position.cdata(), sizeof(float) * 3);
		directionalLight.Pos[3] = 1.0f;

		memcpy(&directionalLight.Color, self.m_color.cdata(), sizeof(float) * 4);
		memcpy(&directionalLight.Sky, self.m_sky.cdata(), sizeof(float) * 4);

		directionalLight.Attenuation = self.m_attenuation;
		directionalLight.AttenuationOffset = self.m_attenuationOffset;
		directionalLight.Range = self.m_range;
		
		if (self.IsShadowEnabled())
		{
			directionalLight.ShadowResolution[0] = directionalLight.ShadowResolution[1] = (float) self.m_shadowResolution;
			directionalLight.ShadowPixel[0] = directionalLight.ShadowPixel[1] = 1.0f / self.m_shadowResolution;

			for (uint4 i = 0; i < DirectionalLightConstantBuffer::MaxShadowSplitCount; ++i)
			{
				directionalLight.ShadowSplitPlanes[i] = perspectiveData.splitPlanes[i];

				const PerspectiveDesc &splitDesc = perspectiveData.pShadowPerspectives[i]->GetDesc();

				memcpy(directionalLight.ShadowSplits[i].Proj, splitDesc.ViewProjMat.cdata(), sizeof(float) * 16);
				directionalLight.ShadowSplits[i].Near = splitDesc.NearPlane;
				directionalLight.ShadowSplits[i].Far = splitDesc.FarPlane;

				directionalLight.ShadowSplits[i].PixelScaleX = perspectiveData.pixelScale[i][0];
				directionalLight.ShadowSplits[i].PixelScaleY = perspectiveData.pixelScale[i][1];
			}
		}

		ToImpl(context.Context())->UpdateSubresource(
			ToImpl(*self.Light::m_pConstants), 0, nullptr,
			&directionalLight, 0, 0);

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
void DirectionalLightController::Flush()
{
	m_position = m_pEntity->GetPosition();
	m_orientation = m_pEntity->GetOrientation();

	SetLocalBounds( beMath::fsphere3( beMath::fvec3(), FLT_MAX * 0.5f) );

	m_bConstantsChanged = true;

	MeshController::Flush();
}

// Gets the reflection properties.
DirectionalLightController::Properties DirectionalLightController::GetControllerProperties()
{
	return Properties(DirectionalLightControllerProperties.data(), DirectionalLightControllerProperties.data_end());
}

// Gets the reflection properties.
DirectionalLightController::Properties DirectionalLightController::GetReflectionProperties() const
{
	return Properties(DirectionalLightControllerProperties.data(), DirectionalLightControllerProperties.data_end());
}

// Gets the controller type.
utf8_ntr DirectionalLightController::GetControllerType()
{
	return utf8_ntr("DirectionalLightController");
}

} // namespace

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/beRenderableMaterialCache.h"

namespace beScene
{

// Creates a directional light.
lean::resource_ptr<DirectionalLightController, true> CreateDirectionalLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return lean::new_resource<DirectionalLightController>(
			pEntity, pScene, pScenery,
			renderer.PipePool(), *resources.MeshCache(), *renderer.Device()
		);
}

namespace
{

/// Default effect.
inline utf8_string& GetDefaultEffectFile()
{
	static utf8_string defaultEffectFile = "Lights/DirectionalLight.fx";
	return defaultEffectFile;
}

} // namespace

// Sets the default directional light effect file.
void SetDirectionalLightDefaultEffect(const utf8_ntri &file)
{
	GetDefaultEffectFile().assign(file.begin(), file.end());
}

// Gets the default directional light effect file.
beCore::Exchange::utf8_string GetDirectionalLightDefaultEffect()
{
	const utf8_string &file = GetDefaultEffectFile();
	return beCore::Exchange::utf8_string(file.begin(), file.end());
}

// Gets the default material for directional lights.
RenderableMaterial* GetDirectionalLightDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return renderer.RenderableMaterials()->GetMaterial(
			resources.MaterialCache()->GetMaterial(
				resources.EffectCache()->GetEffect(GetDefaultEffectFile(), nullptr, 0),
				"beDirectionalLightController.Material"
			)
		);
}

// Creates a directional light using the default effect.
lean::resource_ptr<DirectionalLightController, true> CreateDefaultDirectionalLightController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	lean::resource_ptr<DirectionalLightController> pDirectionalLight = lean::new_resource<DirectionalLightController>(
			pEntity, pScene, pScenery,
			renderer.PipePool(), *resources.MeshCache(), *renderer.Device());

	pDirectionalLight->SetMaterial( GetDirectionalLightDefaultMaterial(resources, renderer) );

	return pDirectionalLight.transfer();
}

} // namespace
