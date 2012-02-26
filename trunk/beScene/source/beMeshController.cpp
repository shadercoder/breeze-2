/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMeshController.h"

#include <beEntitySystem/beEntity.h>
#include "beScene/beSceneController.h"
#include "beScene/beDynamicScenery.h"
#include "beScene/DX11/beMesh.h"
#include "beScene/beMeshCompound.h"

#include "beScene/beRenderableMaterial.h"
#include "beScene/beMaterialSort.h"
#include "beScene/beRenderJob.h"

#include "beScene/beRenderableEffectData.h"
#include "beScene/beAbstractRenderableEffectDriver.h"
#include "beScene/beRenderContext.h"

#include <beGraphics/Any/beSetup.h>
#include <beGraphics/Any/beDevice.h>
#include <beGraphics/Any/beStateManager.h>
#include <beGraphics/Any/beDeviceContext.h>

#include <beGraphics/DX/beError.h>

#include <lean/functional/algorithm.h>

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>
#include <beMath/beSphere.h>

#include <lean/io/numeric.h>

namespace beScene
{

/// Subset.
struct MeshController::Subset
{
	lean::resource_ptr<const Mesh> mesh;
	lean::resource_ptr<const RenderableMaterial> pMaterial;

	/// Default constructor.
	Subset() { }
	/// Constructor.
	Subset(const Mesh *mesh, const RenderableMaterial *pMaterial)
		: mesh(mesh),
		pMaterial(pMaterial) { }
};

/// Pass
struct MeshController::Pass
{
	lean::com_ptr<ID3D11InputLayout> pInputLayout;
	const DX11::Mesh *pMesh;
	lean::resource_ptr<const AbstractRenderableEffectDriver> pEffectDriver;
	const QueuedPass *pPass;
	lean::resource_ptr<const beGraphics::Any::Setup> pSetup;

	/// Constructor.
	Pass(ID3D11InputLayout *pInputLayout,
		const DX11::Mesh *pMesh,
		const AbstractRenderableEffectDriver *pEffectDriver,
		const QueuedPass *pPass,
		const beGraphics::Any::Setup *pSetup)
			: pInputLayout(pInputLayout),
			pMesh(pMesh),
			pEffectDriver(pEffectDriver),
			pPass(pPass),
			pSetup(pSetup) { }
};

/// Shared data structure.
struct MeshController::SharedData : RenderableEffectData { };

namespace
{

/// External pass data structure.
struct PassData
{
	ID3D11InputLayout *pInputLayout;
	const DX11::Mesh *pMesh;
	const AbstractRenderableEffectDriver *pEffectDriver;
	const QueuedPass *pPass;
	const beGraphics::Any::Setup *pSetup;
};

/// Builds a list of passes from the given list of mesh subsets.
MeshController::pass_vector CollectPasses(const MeshController::Subset *subsets, uint4 subsetCount)
{
	MeshController::pass_vector result;

	typedef std::vector<MeshController::Pass> pass_vector;
	pass_vector passes;
	passes.reserve(2 * subsetCount);

	for (uint4 subsetIdx = 0; subsetIdx < subsetCount; ++subsetIdx)
	{
		const MeshController::Subset &subset = subsets[subsetIdx];

		if (subset.pMaterial)
		{
			const DX11::Mesh& mesh = ToImpl(*subset.mesh);

			const uint4 techniqueCount = subset.pMaterial->GetTechniqueCount();

			for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
			{
				const RenderableMaterialTechnique &technique = subset.pMaterial->GetTechnique(techniqueIdx);
				const AbstractRenderableEffectDriver *pEffectBinder = technique.EffectDriver;

				const uint4 passCount = pEffectBinder->GetPassCount();

				for (uint4 passID = 0; passID < passCount; ++passID)
				{
					const QueuedPass *pPass = pEffectBinder->GetPass(passID);

					uint4 passSignatureSize = 0;
					const char *pPassSignature = pPass->GetInputSignature(passSignatureSize);

					lean::com_ptr<ID3D11InputLayout> pInputLayout;
					BE_THROW_DX_ERROR_MSG(
						beGraphics::Any::GetDevice(*mesh.GetVertexBuffer())->CreateInputLayout(
							mesh.GetVertexElementDescs(),
							mesh.GetVertexElementDescCount(),
							pPassSignature, passSignatureSize,
							pInputLayout.rebind()),
						"ID3D11Device::CreateInputLayout()");

					passes.push_back( MeshController::Pass(pInputLayout, &mesh, pEffectBinder, pPass, ToImpl(technique.Setup)) );
				}
			}
		}
	}

	result.assign_disjoint(passes.begin(), passes.end(), MeshController::pass_vector::consume);
	return result;
}

/// Computes the bounds of the given list of subsets.
beMath::fsphere3 ComputeBounds(const MeshController::Subset *subsets, uint4 subsetCount)
{
	beMath::fsphere3 bounds;

	if (subsetCount > 1)
	{
		beMath::fvec3 boundsMin(FLT_MAX);
		beMath::fvec3 boundsMax(-FLT_MAX);

		for (uint4 subsetIdx = 0; subsetIdx < subsetCount; ++subsetIdx)
		{
			const beMath::fsphere3 &subsetBounds = subsets[subsetIdx].mesh->GetBounds();

			boundsMin = min_cw(boundsMin, subsetBounds.p() - subsetBounds.r());
			boundsMin = max_cw(boundsMax, subsetBounds.p() + subsetBounds.r());
		}

		bounds.p() = (boundsMin + boundsMax) * 0.5f;
		bounds.r() = length(boundsMax - boundsMin) * 0.5f;
	}
	else if (subsetCount == 1)
		bounds = subsets->mesh->GetBounds();

	return bounds;
}

} // namespace

// Constructor.
MeshController::MeshController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery)
	: EntityController(pEntity, pScene),
	m(pScenery)
{
}

// Constructor.
MeshController::M::M(DynamicScenery *pScenery)
	: pScenery( LEAN_ASSERT_NOT_NULL(pScenery) ),
	pSharedData(),
	pBounds(),
	bRendered(false),
	bSynchronized(false),
	bVisible(true)
{
}

// Destructor.
MeshController::~MeshController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Gets the number of subsets.
uint4 MeshController::GetSubsetCount() const
{
	return static_cast<uint4>( m.subsets.size() );
}

// Gets the n-th mesh.
const Mesh* MeshController::GetMesh(uint4 subsetIdx) const
{
	return (subsetIdx < m.subsets.size())
		? m.subsets[subsetIdx].mesh
		: nullptr;
}

namespace
{

/// Updates the mesh controller when attached to a scenery.
void UpdateInScenery(MeshController::M &m)
{
	// Rebuild enclosing sphere
	m.localBounds = ComputeBounds(&m.subsets.front(), m.subsets.size());

	// Rebuild list of passes
	// WARNING: Some external pass data pointers might dangle, ...
	m.passes.assign( CollectPasses(&m.subsets.front(), m.subsets.size()), MeshController::pass_vector::consume );
	
	// Update renderable data
	// WARNING: ... however, pass data is accessed nowhere but in Render()
	if (m.bRendered)
		m.pScenery->InvalidateRenderables();
}

} // namespace

// Adds the given mesh setting the given material.
uint4 MeshController::AddMeshWithMaterial(const Mesh *mesh, const RenderableMaterial *pMaterial)
{
	if (!mesh)
	{
		LEAN_LOG_ERROR_MSG("mesh may not be nullptr");
		return static_cast<uint4>(-1);
	}

	uint4 subsetIdx = static_cast<uint4>(m.subsets.size());
	m.subsets.push_back( Subset(mesh, pMaterial) );

	// Update renderable
	if (m.bRendered)
		UpdateInScenery(m);

	EmitPropertyChanged();

	return subsetIdx;
}

// Sets the n-th mesh.
void MeshController::SetMeshWithMaterial(uint4 subsetIdx, const Mesh *mesh, const RenderableMaterial *pMaterial)
{
	if (subsetIdx < m.subsets.size())
	{
		Subset &subset = m.subsets[subsetIdx];

		if (mesh)
			subset.mesh = mesh;
		if (pMaterial)
			subset.pMaterial = pMaterial;

		// Update renderable
		if (m.bRendered)
			UpdateInScenery(m);

		EmitPropertyChanged();
	}
	else
		AddMeshWithMaterial(mesh, pMaterial);
}

// Removes the given mesh with the given material.
void MeshController::RemoveMeshWithMaterial(const Mesh *pMesh, const RenderableMaterial *pMaterial)
{
	bool bRemoved = false;

	subset_vector::iterator it = m.subsets.begin();

	while (it != m.subsets.end())
		if (it->mesh == pMesh && (it->pMaterial == pMaterial || !pMaterial))
		{
			it = m.subsets.erase(it);
			bRemoved = true;
		}
		else
			++it;
	
	if (bRemoved)
	{
		// Update renderable
		if (m.bRendered)
			UpdateInScenery(m);

		EmitPropertyChanged();
	}
}

// Removes the n-th subset.
void MeshController::RemoveSubset(uint4 subsetIdx)
{
	if (subsetIdx < m.subsets.size())
	{
		m.subsets.erase( m.subsets.begin() + subsetIdx );

		// Update renderable
		if (m.bRendered)
			UpdateInScenery(m);

		EmitPropertyChanged();
	}
}

// Sets the material.
void MeshController::SetMaterial(const RenderableMaterial *pMaterial)
{
	if (pMaterial)
	{
		for (subset_vector::iterator it = m.subsets.begin(); it != m.subsets.end(); ++it)
			it->pMaterial = pMaterial;
		
		// Update renderable
		if (m.bRendered)
			UpdateInScenery(m);

		EmitPropertyChanged();
	}
}

// Sets the n-th material.
void MeshController::SetMaterial(uint4 subsetIdx, const RenderableMaterial *pMaterial)
{
	SetMeshWithMaterial(subsetIdx, nullptr, pMaterial);
}

// Gets the n-th material.
const RenderableMaterial* MeshController::GetMaterial(uint4 subsetIdx) const
{
	return (subsetIdx < m.subsets.size())
		? m.subsets[subsetIdx].pMaterial
		: nullptr;
}

// Gets the sort index.
uint4 MeshController::GetSortIndex() const
{
	return (!m.passes.empty())
		? MaterialSort::GenerateSortIndex(m.passes.front().pPass, m.passes.front().pSetup.get())
		: 0;
}

// Gets the number of passes.
uint4 MeshController::GetPassCount() const
{
	return static_cast<uint4>( m.passes.size() );
}

// Called when a renderable is attached to a scenery.
void MeshController::Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator)
{
	data.Render = &MeshController::Render;

	// Store pointer to bounds for synchronization
	m.pBounds = &data.Bounds;

	// Store pointer to shared data for synchronization
	m.pSharedData = allocator.AllocateRenderableData<SharedData>();
	data.SharedData = m.pSharedData;

	LEAN_ASSERT( data.PassCount == m.passes.size() );

	for (uint4 passIdx = 0; passIdx < data.PassCount; ++passIdx)
	{
		const Pass &pass = m.passes[passIdx];
		RenderablePass &renderablePass = data.Passes[passIdx];

		renderablePass.PassData = nullptr;
		renderablePass.StageID = pass.pPass->GetStageID();
		renderablePass.QueueID = pass.pPass->GetQueueID();
		renderablePass.SortIndex = MaterialSort::GenerateSortIndex(pass.pPass, pass.pSetup.get());
		renderablePass.Flags = 0; // TODO: Lit
	}

	// Data pointers valid, start synchronizing
	if (!m.bSynchronized)
	{
		m_pScene->AddSynchronized(this, beEntitySystem::SynchronizedFlags::Flush);
		m.bSynchronized = true;
	}
}

// Called for each pass when a renderable is attached to a scenery.
void MeshController::Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &renderablePass, uint4 passIdx, RenderableDataAllocator &allocator)
{
	const Pass &pass = m.passes[passIdx];

	// Copy pass data to scenery data store
	PassData *pPassData = allocator.AllocateRenderablePassData<PassData>();
	pPassData->pInputLayout = pass.pInputLayout;
	pPassData->pMesh = pass.pMesh;
	pPassData->pEffectDriver = pass.pEffectDriver;
	pPassData->pPass = pass.pPass;
	pPassData->pSetup = pass.pSetup;

	renderablePass.PassData = pPassData;
}

// Called when a renderable is detached from a scenery.
void MeshController::Detached(DynamicScenery *pScenery)
{
	if (m.pSharedData)
	{
		m.pBounds = nullptr;
		m.pSharedData = nullptr;
	}

	// Data pointers invalid, stop synchronizing
	if (m.bSynchronized)
	{
		m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);
		m.bSynchronized = false;
	}
}

// Renders the mesh
void MeshController::Render(const RenderJob &job, const Perspective &perspective, 
	const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context)
{
	const SharedData &renderableData = *static_cast<const SharedData*>(job.SharedData);
	const PassData &passData = *static_cast<const PassData*>(job.PassData);

	ID3D11DeviceContext *pContext = ToImpl(context.Context());
	beGraphics::Any::StateManager &stateManager = ToImpl(context.StateManager());
	const DX11::Mesh &mesh = *passData.pMesh;

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetInputLayout(passData.pInputLayout);

	ID3D11Buffer *pVertexBuffer = mesh.GetVertexBuffer().GetBuffer();
	UINT vertexStride =  mesh.GetVertexSize();
	UINT vertexOffset = 0;
	pContext->IASetVertexBuffers(0, 1, &mesh.GetVertexBuffer().GetBuffer(), &vertexStride, &vertexOffset);
	pContext->IASetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndexFormat(), 0);

	stateManager.Revert();

	passData.pSetup->Apply(context.Context());
	passData.pEffectDriver->Apply(&renderableData, perspective, context.StateManager(), context.Context());

	RenderableDriverState driverState;

	for (uint4 i = 0;
		passData.pEffectDriver->ApplyPass(passData.pPass, i, &renderableData, perspective, lights, lightsEnd, driverState, context.StateManager(), context.Context());
		)
	{
		stateManager.Reset();

		pContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
	}
}

// Synchronizes this controller with the controlled entity.
void MeshController::Synchronize()
{
}

// Synchronizes this controller with the controlled entity.
void MeshController::Flush()
{
	LEAN_ASSERT_NOT_NULL(m.pBounds);
	LEAN_ASSERT_NOT_NULL(m.pSharedData);

	m.pSharedData->ID = m_pEntity->GetID();

	const beMath::fvec3 &pos = m_pEntity->GetPosition();
	const beMath::fmat3 &orientation = m_pEntity->GetOrientation();
	const beMath::fvec3 &scaling = m_pEntity->GetScaling();

	m.pSharedData->Transform = mat_transform(pos, orientation[2] * scaling[2], orientation[1] * scaling[1], orientation[0] * scaling[0]);
	m.pSharedData->TransformInv = mat_transform_inverse(pos, orientation[2] / scaling[2], orientation[1] / scaling[1], orientation[0] / scaling[0]);

	m.pBounds->p() = m.localBounds.p() + pos;
	// MONITOR: Hide by ensuring that mesh is always culled
	m.pBounds->r() = (m.bVisible)
		? m.localBounds.r() * max( max(scaling[0], scaling[1]), scaling[2] )
		: -FLT_MAX * 0.5f;
}

// Attaches this controller to the scenery.
void MeshController::Attach()
{
	if (!m.bRendered)
	{
		UpdateInScenery(m);

		m.pScenery->AddRenderable(this);
		m.bRendered = true;
	}
}

// Detaches this controller from the scenery.
void MeshController::Detach()
{
	if (m.bRendered)
	{
		m.pScenery->RemoveRenderable(this);
		m.bRendered = false;
	}
}

// Gets the number of child components.
uint4 MeshController::GetComponentCount() const
{
	return static_cast<uint4>( m.subsets.size() );
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string MeshController::GetComponentName(uint4 idx) const
{
	beCore::Exchange::utf8_string name;

	// TODO: Read mesh names somewhere? (Mesh compound!) :-/

	// Ugly default names
	utf8_string num = lean::int_to_string(idx);
	name.reserve(lean::ntarraylen("Subset ") + num.size());
	name.append("Subset ");
	name.append(num.c_str(), num.c_str() + num.size());
	
	return name;
	}

	// Gets the n-th reflected child component, nullptr if not reflected.
	const beCore::ReflectedComponent* MeshController::GetReflectedComponent(uint4 idx) const
	{
		if (idx < m.subsets.size())
	{
		const RenderableMaterial *pMaterial = m.subsets[idx].pMaterial;
		return (pMaterial) ? pMaterial->GetMaterial() : nullptr;
	}
	else
		return nullptr;
}

// Gets the type of the n-th child component.
beCore::Exchange::utf8_string MeshController::GetComponentType(uint4 idx) const
{
	return "RenderableMaterial";
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> MeshController::GetComponent(uint4 idx) const
{
	return lean::any_value<RenderableMaterial*>( const_cast<RenderableMaterial*>( GetMaterial(idx) ) );
}

// Returns true, if the n-th component can be replaced.
bool MeshController::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void MeshController::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetMaterial( idx, any_cast<RenderableMaterial*>(pComponent) );
}

// Gets the controller type.
utf8_ntr MeshController::GetControllerType()
{
	return utf8_ntr("MeshController");
}

// Adds all meshes in the given mesh compound to the given mesh controller using the given material.
void AddMeshes(MeshController &controller, const MeshCompound &compound, const RenderableMaterial *pMaterial)
{
	const uint4 meshCount = compound.GetSubsetCount();

	for (uint4 i = 0; i < meshCount; ++i)
		controller.AddMeshWithMaterial(compound.GetSubset(i), pMaterial);
}

} // namespace

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/beRenderableMaterialCache.h"

namespace beScene
{

namespace
{

/// Default effect.
inline utf8_string& GetDefaultEffectFile()
{
	static utf8_string defaultEffectFile = "Materials/Simple.fx";
	return defaultEffectFile;
}

} // namespace

// Sets the default mesh effect file.
void SetMeshDefaultEffect(const utf8_ntri &file)
{
	GetDefaultEffectFile().assign(file.begin(), file.end());
}

// Gets the default mesh effect file.
beCore::Exchange::utf8_string GetMeshDefaultEffect()
{
	const utf8_string &file = GetDefaultEffectFile();
	return beCore::Exchange::utf8_string(file.begin(), file.end());
}

// Gets the default material for meshes.
RenderableMaterial* GetMeshDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return renderer.RenderableMaterials()->GetMaterial(
			resources.MaterialCache()->GetMaterial(
				resources.EffectCache()->GetEffect(GetDefaultEffectFile(), nullptr, 0),
				"beMeshController.Material"
			)
		);
}

} // namespace
