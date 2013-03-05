/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMeshControllerSerializer.h"
#include "beScene/beMeshControllers.h"
#include "beScene/beRenderingController.h"

#include <beEntitySystem/beWorld.h>
#include <beEntitySystem/beSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include "beScene/beSerializationParameters.h"
#include <beCore/beParameterSet.h>
#include <beCore/beParameters.h>
#include <beCore/beExchangeContainers.h>

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/beRenderableMaterialCache.h"
#include "beScene/beRenderableMeshCache.h"
#include "beScene/beInlineMeshSerialization.h"

#include "beScene/beRenderableMesh.h"

#include <lean/xml/numeric.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beScene
{

MeshControllers* CreateMeshControllers(const beCore::ParameterSet &parameters)
{
	bees::EntitySystemParameters entityParameters = bees::GetEntitySystemParameters(parameters);
	bees::World *world = LEAN_THROW_NULL(entityParameters.World);

	MeshControllers *pMeshes = static_cast<MeshControllers*>(
			world->Controllers().GetController(MeshControllers::GetComponentType())
		);
	
	if (!pMeshes)
	{
		SceneParameters sceneParameters = GetSceneParameters(parameters);
		RenderingController *renderingCtrl = LEAN_THROW_NULL(sceneParameters.RenderingController);

		pMeshes = CreateMeshControllers(&world->PersistentIDs()).detach();
		pMeshes->SetComponentMonitor(sceneParameters.ResourceManager->Monitor());
		world->Controllers().AddControllerKeep(pMeshes);
		renderingCtrl->AddRenderable(pMeshes);
	}

	return pMeshes;
}

// Gets the serialization parameter IDs.
MeshControllers* GetMeshControllers(const beCore::ParameterSet &parameters, bool bMutable = false)
{
	static uint4 meshControllerParameterID = bees::GetSerializationParameters().Add("beScene.MeshControllers");

	const beCore::ParameterLayout &layout = bees::GetSerializationParameters();
	MeshControllers *pMeshes = parameters.GetValueDefault< MeshControllers* >(layout, meshControllerParameterID);

	if (!pMeshes)
	{
		pMeshes = CreateMeshControllers(parameters);
		if (bMutable)
			const_cast<beCore::ParameterSet&>(parameters).SetValue< MeshControllers* >(layout, meshControllerParameterID, pMeshes);
	}

	return pMeshes;
}

// Constructor.
MeshControllerSerializer::MeshControllerSerializer()
	: ControllerSerializer(MeshController::GetComponentType()->Name)
{
}

// Destructor.
MeshControllerSerializer::~MeshControllerSerializer()
{
}

// Gets a list of creation parameters.
beCore::ComponentParameters MeshControllerSerializer::GetCreationParameters() const
{
	static const beCore::ComponentParameter parameters[] = {
			beCore::ComponentParameter( utf8_ntr("Mesh"), RenderableMesh::GetComponentType() )
		};
	return beCore::ComponentParameters(parameters, parameters + lean::arraylen(parameters));
}

// Creates a serializable object from the given parameters.
lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> MeshControllerSerializer::Create(
	const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	MeshControllers* meshControllers = GetMeshControllers(parameters);

	// Get parameters
	RenderableMesh *mesh = creationParameters.GetValueChecked<RenderableMesh*>("Mesh");
	
	// Create controller
	lean::scoped_ptr<MeshController> controller( meshControllers->AddController() );
	controller->SetMesh(mesh);

	return controller.transfer();
}

// Loads a mesh controller from the given xml node.
lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> MeshControllerSerializer::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	SceneParameters sceneParameters = GetSceneParameters(parameters);
	MeshControllers* meshControllers = GetMeshControllers(parameters);

	lean::resource_ptr<RenderableMesh> renderableMesh;

	// Check for legacy file format
	utf8_ntr mainMaterialName = lean::get_attribute(node, "materialName");

	if (mainMaterialName.empty())
	{
		renderableMesh = sceneParameters.Renderer->RenderableMeshes->GetByName( lean::get_attribute(node, "mesh"), true );
		// Fill new/unconfigured subsets with default material
		FillRenderableMesh(
				*renderableMesh,
				GetMeshDefaultMaterial(*sceneParameters.ResourceManager, *sceneParameters.Renderer),
				*sceneParameters.Renderer->RenderableMaterials
			);
		CacheMaterials(*renderableMesh, *sceneParameters.ResourceManager->MaterialCache);
	}
	else
	{
		beg::Material *mainMaterial = sceneParameters.ResourceManager->MaterialCache->GetByName( mainMaterialName, true );
		AssembledMesh *mainMesh = sceneParameters.ResourceManager->MeshCache->GetByFile( lean::get_attribute(node, "mesh") );

		renderableMesh = ToRenderableMesh(*mainMesh, nullptr, true);

		uint4 nextSubsetIdx = 0;

		// Apply materials
		for (const rapidxml::xml_node<utf8_t> *subsetNode = node.first_node();
			subsetNode; subsetNode = subsetNode->next_sibling())
		{
			uint4 subsetIdx = lean::get_int_attribute(*subsetNode, "subset", nextSubsetIdx);
			nextSubsetIdx = nextSubsetIdx + 1;
			
			utf8_ntr materialName = lean::get_attribute(*subsetNode, "materialName");
			// Empty material means main material
			if (materialName.empty())
				continue;

			if (subsetIdx < Size(renderableMesh->GetMeshes()))
			{
				beg::Material *material = sceneParameters.ResourceManager->MaterialCache->GetByName( materialName, true );
				renderableMesh->SetMeshWithMaterial( subsetIdx,
						nullptr,
						sceneParameters.Renderer->RenderableMaterials->GetMaterial(material)
					);
			}
			else
				LEAN_LOG_ERROR_CTX("MeshController subset out of bounds, information will be lost", materialName);
		}

		// Fill in missing materials
		for (uint4 subsetIdx = 0, subsetCount = Size(renderableMesh->GetMeshes()); subsetIdx < subsetCount; ++subsetIdx)
			if (!renderableMesh->GetMaterials()[subsetIdx])
				renderableMesh->SetMeshWithMaterial(subsetIdx, nullptr, sceneParameters.Renderer->RenderableMaterials->GetMaterial(mainMaterial));

		// Add to shared resource environment
		sceneParameters.Renderer->RenderableMeshes->SetName(
				renderableMesh,
				sceneParameters.Renderer->RenderableMeshes->GetUniqueName( bec::GetCachedName<utf8_ntr>(mainMesh) )
			);
	}

	lean::scoped_ptr<MeshController> controller( meshControllers->AddController() );
	controller->SetMesh(renderableMesh);
	ControllerSerializer::Load(controller.get(), node, parameters, queue);

	return controller.transfer();
}

// Saves the given mesh controller to the given XML node.
void MeshControllerSerializer::Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const
{
	ControllerSerializer::Save(pSerializable, node, parameters, queue);

	const MeshController &meshController = static_cast<const MeshController&>(*pSerializable);
	
	if (const RenderableMesh *pRenderableMesh = meshController.GetMesh())
	{
		utf8_ntr name = bec::GetCachedName<utf8_ntr>(pRenderableMesh);
		if (!name.empty())
			lean::append_attribute( *node.document(), node, "mesh", name );
		else
			LEAN_LOG_ERROR_MSG("Could not identify MeshController mesh, information will be lost");

		SaveMesh(pRenderableMesh, parameters, queue);
	}
}

const beEntitySystem::EntityControllerSerializationPlugin<MeshControllerSerializer> MeshControllerSerialization;

} // namespace
