/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMeshControllerSerializer.h"
#include "beScene/beMeshController.h"

#include <beEntitySystem/beControllerSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include "beScene/beSerializationParameters.h"
#include <beCore/beParameterSet.h>
#include <beCore/beExchangeContainers.h>

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/beRenderableMaterialCache.h"
#include "beScene/beInlineMaterialSerialization.h"

#include "beScene/beRenderableMaterial.h"
#include "beScene/beMaterial.h"
#include "beScene/beMesh.h"

#include "beScene/beSceneController.h"

#include "beScene/beMaterialCache.h"
#include "beScene/beMeshCache.h"

#include <lean/xml/numeric.h>

#include <lean/logging/log.h>

namespace beScene
{

// Constructor.
MeshControllerSerializer::MeshControllerSerializer()
	: ControllerSerializer(MeshController::GetControllerType())
{
}

// Destructor.
MeshControllerSerializer::~MeshControllerSerializer()
{
}

// Gets a list of creation parameters.
MeshControllerSerializer::SerializationParameters MeshControllerSerializer::GetCreationParameters() const
{
	static const beEntitySystem::CreationParameter parameters[] = {
			beEntitySystem::CreationParameter( utf8_ntr("Mesh"), utf8_ntr("Mesh") ),
			beEntitySystem::CreationParameter( utf8_ntr("Material"), utf8_ntr("RenderableMaterial"), true )
		};

	return SerializationParameters(parameters, parameters + lean::arraylen(parameters));
}

// Creates a serializable object from the given parameters.
lean::resource_ptr<beEntitySystem::Controller, true> MeshControllerSerializer::Create(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	SceneParameters sceneParameters = GetSceneParameters(parameters);

	// Get mesh
	const MeshCompound &mesh = *creationParameters.GetValueChecked<const MeshCompound*>("Mesh");

	// (Optionally) get material
	const RenderableMaterial *pMaterial = creationParameters.GetValueDefault<const RenderableMaterial*>("Material");

	// Default material, if none specified
	if (!pMaterial)
		pMaterial = GetMeshDefaultMaterial(*sceneParameters.ResourceManager, *sceneParameters.Renderer);
	
	lean::resource_ptr<MeshController> pController = lean::new_resource<MeshController>(
			entityParameters.Entity, sceneParameters.SceneController, sceneParameters.Scenery
		);
	AddMeshes(*pController, mesh, pMaterial);

	return pController.transfer();
}

namespace
{

/// Gets a mesh from the given cache.
inline MeshCompound* GetMesh(const rapidxml::xml_node<lean::utf8_t> &node, MeshCache &cache)
{
	utf8_ntr file = lean::get_attribute(node, "mesh");

	if (!file.empty())
		return cache.GetMesh(file);
	else
	{
		utf8_ntr name = lean::get_attribute(node, "meshName");

		if (!name.empty())
			return cache.GetMeshByName(name, true);
	}

	return nullptr;
}

/// Gets a material from the given cache.
inline Material* GetMaterial(const rapidxml::xml_node<lean::utf8_t> &node, MaterialCache &cache)
{
	utf8_ntr file = lean::get_attribute(node, "material");

	if (!file.empty())
		return cache.GetMaterial(file);
	else
	{
		utf8_ntr name = lean::get_attribute(node, "materialName");

		if (!name.empty())
			return cache.GetMaterialByName(name, true);
	}

	return nullptr;
}

} // namespace

// Loads a mesh controller from the given xml node.
lean::resource_ptr<beEntitySystem::Controller, true> MeshControllerSerializer::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::LoadJob> &queue) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	SceneParameters sceneParameters = GetSceneParameters(parameters);

	lean::resource_ptr<MeshController> pController = lean::new_resource<MeshController>(
		entityParameters.Entity, sceneParameters.SceneController, sceneParameters.Scenery );
	
	ControllerSerializer::Load(pController, node, parameters, queue);

	// Load main mesh & material
	MeshCompound *pMainMesh = GetMesh(node, *sceneParameters.ResourceManager->MeshCache());
	Material *pMainMaterial = GetMaterial(node, *sceneParameters.ResourceManager->MaterialCache());
	
	RenderableMaterial *pRenderableMainMaterial = (pMainMaterial) ? sceneParameters.Renderer->RenderableMaterials()->GetMaterial(pMainMaterial) : nullptr;

	if (pMainMesh)
	{
		uint4 implicitSubsetCount = pMainMesh->GetSubsetCount();
		implicitSubsetCount = min( lean::get_int_attribute(node, "implicitSubsets",  implicitSubsetCount), implicitSubsetCount );

		// Add implicit subsets
		for (uint4 subsetIdx = 0; subsetIdx < implicitSubsetCount; ++subsetIdx)
			pController->AddMeshWithMaterial( pMainMesh->GetSubset(subsetIdx), pRenderableMainMaterial );

		// TODO: Also print persistent ID
		if (implicitSubsetCount > 0 && !pMainMaterial)
			LEAN_LOG_ERROR("MeshController #TODO is missing a main material specification, some or all implicit subsets might be invisible");
	}

	uint4 nextSubsetIdx = 0;

	// Add additional subsets & apply material modifications
	for (const rapidxml::xml_node<utf8_t> *pSubsetNode = node.first_node();
		pSubsetNode; pSubsetNode = pSubsetNode->next_sibling())
	{
		uint4 subsetIdx = lean::get_int_attribute(*pSubsetNode, "subset", nextSubsetIdx);

		// Load individual submesh & material
		MeshCompound *pMesh = GetMesh(*pSubsetNode, *sceneParameters.ResourceManager->MeshCache());
		Material *pMaterial = GetMaterial(*pSubsetNode, *sceneParameters.ResourceManager->MaterialCache());

		RenderableMaterial *pRenderableMaterial;

		// Default to main mesh & material
		if (!pMesh)
			pMesh = pMainMesh;
		if (!pMaterial)
		{
			pMaterial = pMainMaterial;
			pRenderableMaterial = pRenderableMainMaterial;
		}
		else
			pRenderableMaterial = sceneParameters.Renderer->RenderableMaterials()->GetMaterial(pMaterial);
		
		if (pMesh)
		{
			const uint4 submeshCount = pMesh->GetSubsetCount();

			uint4 submeshIdx = lean::get_int_attribute(*pSubsetNode, "submesh", 0);
			
			if (submeshIdx < submeshCount)
			{
				if (!pMaterial)
					LEAN_LOG_ERROR("MeshController #TODO subset " << subsetIdx << " is missing a material specification, will be invisible");

				// Apply modifications (TODO: requires subset idx sorting!)
				pController->SetMeshWithMaterial(subsetIdx, pMesh->GetSubset(submeshIdx), pRenderableMaterial);
			}
			else
				LEAN_LOG_ERROR("MeshController #TODO subset " << subsetIdx << ": submesh " << submeshIdx << " out of range, will be lost");
		}
		else
			LEAN_LOG_ERROR("MeshController #TODO subset " << subsetIdx << " is missing a mesh specification, will be lost");

		nextSubsetIdx = subsetIdx + 1;
	}

	return pController.transfer();
}

namespace
{

/// Gets the material, if available.
LEAN_INLINE const Material* MaybeGetMaterial(const RenderableMaterial *pMaterial)
{
	return (pMaterial) ? pMaterial->GetMaterial() : nullptr;
}
/// Gets the file, if available.
template <class Resource, class ResourceCache>
LEAN_INLINE beCore::Exchange::utf8_string MaybeGetFile(const Resource *pResource, const ResourceCache *pCache, bool &bIsFile)
{
	beCore::Exchange::utf8_string result;
	
	utf8_ntr file = (pCache) ? pCache->GetFile(pResource, &bIsFile) : utf8_ntr("");

	if (pCache && bIsFile)
		result = pCache->GetPathResolver().Shorten(file);
	else
		result.assign(file.begin(), file.end());

	return result;
}
/// Gets the file, if available.
template <class Resource>
LEAN_INLINE beCore::Exchange::utf8_string MaybeGetFile(const Resource *pResource, bool &bIsFile)
{
	return MaybeGetFile(pResource, (pResource) ? pResource->GetCache() : nullptr, bIsFile);
}

// Saves the given resource in one of the given attributes. Returns true, if saved by name, false otherwise.
template <class Resource>
inline bool AppendResourceAttribute(rapidxml::xml_node<lean::utf8_t> &node, const utf8_ntri &fileAtt, const utf8_ntri &nameAtt, const Resource *pResource)
{
	bool bFile = false;
	beCore::Exchange::utf8_string file = MaybeGetFile(pResource, bFile);

	if (bFile)
	{
		lean::append_attribute(*node.document(), node, fileAtt, file);
		return false;
	}
	else if (!file.empty())
	{
		lean::append_attribute(*node.document(), node, nameAtt, file);
		return true;
	}
	else
		// TODO: Also print persistent ID
		LEAN_LOG_ERROR("Could not identify MeshController #TODO resource '" << fileAtt.c_str() << "', will be lost");

	return false;
}

} // namespace

// Saves the given mesh controller to the given XML node.
void MeshControllerSerializer::Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::SaveJob> &queue) const
{
	ControllerSerializer::Save(pSerializable, node, parameters, queue);

	const MeshController &meshController = static_cast<const MeshController&>(*pSerializable);
	const uint4 subsetCount = meshController.GetSubsetCount();

	const DynamicScenery *pScenery = meshController.GetScenery();

/*	// TODO: Optionally serialize custom scenery
	if (pScenery != meshController.GetScene()->GetScenery())
		lean::append_int_attribute(*node.document(), node, "scenery", pScenery->GetPersistentID());
*/	
	if (subsetCount > 0)
	{
		const MeshCompound *pMainMesh = meshController.GetMesh(0)->GetCompound();
		const Material *pMainMaterial = MaybeGetMaterial(meshController.GetMaterial(0));

		// Store main mesh & material
		AppendResourceAttribute(node, "mesh", "meshName", pMainMesh);
		if (AppendResourceAttribute(node, "material", "materialName", pMainMaterial))
			SaveMaterial(pMainMaterial, parameters, queue);

		uint4 implicitSubsetCount = (pMainMesh) ? pMainMesh->GetSubsetCount() : 0;

		// Store additional subsets, removed subsets & material modifications
		for (uint4 subsetIdx = 0; subsetIdx < subsetCount; ++subsetIdx)
		{
			const Mesh *pSubset = meshController.GetMesh(subsetIdx);
			
			const MeshCompound *pMesh = pSubset->GetCompound();
			const Material *pMaterial = MaybeGetMaterial(meshController.GetMaterial(subsetIdx));

			// ASSERT: subsetIdx < implicitSubsetCount => pMainMesh != nullptr
			if (subsetIdx < implicitSubsetCount && (pMesh != pMainMesh || pSubset != pMainMesh->GetSubset(subsetIdx)))
				// Stop adding implicit subsets at this index
				implicitSubsetCount = subsetIdx;

			// Add, if explicit subset or custom material
			if (subsetIdx >= implicitSubsetCount || pMaterial != pMainMaterial)
			{
				rapidxml::xml_node<utf8_t> &subsetNode = *lean::allocate_node<utf8_t>(*node.document(), "s");
				// ORDER: Append FIRST, otherwise parent document == nullptrs
				node.append_node(&subsetNode);

				lean::append_int_attribute(*subsetNode.document(), subsetNode, "subset", subsetIdx);

				if (pMesh != pMainMesh)
					AppendResourceAttribute(subsetNode, "mesh", "meshName", pMesh);
				if (pMaterial != pMainMaterial)
					if (AppendResourceAttribute(subsetNode, "material", "materialName", pMaterial))
						SaveMaterial(pMaterial, parameters, queue);

				if (pMesh)
				{
					const uint4 meshCount = pMesh->GetSubsetCount();

					for (uint4 i = 0; i < meshCount; ++i)
						if (pSubset == pMesh->GetSubset(i))
						{
							lean::append_int_attribute(*subsetNode.document(), subsetNode, "submesh", i);
							break;
						}
				}

				// TODO: In-place material serialization
				// TODO: File rooting
			}
		}

		// Store implicit subset count to allow for complex compositions
		if (pMainMesh && implicitSubsetCount != pMainMesh->GetSubsetCount())
			lean::append_int_attribute(*node.document(), node, "implicitSubsets", implicitSubsetCount);
	}
}

const beEntitySystem::ControllerSerializationPlugin<MeshControllerSerializer> MeshControllerSerialization;

} // namespace
