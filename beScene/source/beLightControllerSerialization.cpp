/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beLightControllerSerializer.h"
#include "beScene/beLightControllers.h"
#include "beScene/beRenderingController.h"

#include <beEntitySystem/beWorld.h>
#include <beEntitySystem/beSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include "beScene/beSerializationParameters.h"
#include <beCore/beParameterSet.h>
#include <beCore/beParameters.h>

#include "beScene/beLightMaterial.h"
#include "beScene/beLightMaterialCache.h"
#include "beScene/beInlineMaterialSerialization.h"

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"

#include <lean/xml/numeric.h>
#include <lean/logging/errors.h>

namespace beScene
{

namespace
{

template <class ControllerType>
LightControllers<ControllerType>* CreateLightControllers(const beCore::ParameterSet &parameters)
{
	typedef LightControllers<ControllerType> LightControllers;

	bees::EntitySystemParameters entityParameters = bees::GetEntitySystemParameters(parameters);
	bees::World *world = LEAN_THROW_NULL(entityParameters.World);

	LightControllers *pLights = static_cast<LightControllers*>(
			world->Controllers().GetController(LightControllers::GetComponentType())
		);
	
	if (!pLights)
	{
		SceneParameters sceneParameters = GetSceneParameters(parameters);
		RenderingController *renderingCtrl = LEAN_THROW_NULL(sceneParameters.RenderingController);

		pLights = CreateLightControllers<ControllerType>(
				&world->PersistentIDs(),
				sceneParameters.Renderer->PerspectivePool(), *sceneParameters.Renderer->Pipeline(),
				*sceneParameters.Renderer->Device()
			).detach();
		pLights->SetComponentMonitor(sceneParameters.ResourceManager->Monitor());
		world->Controllers().AddControllerKeep(pLights);
		renderingCtrl->AddRenderable(pLights);
	}

	return pLights;
}

// Gets the serialization parameter IDs.
template <class ControllerType>
LightControllers<ControllerType>* GetLightControllers(const beCore::ParameterSet &parameters, bool bMutable = false)
{
	typedef LightControllers<ControllerType> LightControllers;

	static uint4 lightControllerParameterID = bees::GetSerializationParameters().Add("beScene." + utf8_string(LightControllers::GetComponentType()->Name));

	const beCore::ParameterLayout &layout = bees::GetSerializationParameters();
	LightControllers *pLights = parameters.GetValueDefault< LightControllers* >(layout, lightControllerParameterID);

	if (!pLights)
	{
		pLights = CreateLightControllers<ControllerType>(parameters);
		if (bMutable)
			const_cast<beCore::ParameterSet&>(parameters).SetValue< LightControllers* >(layout, lightControllerParameterID, pLights);
	}

	return pLights;
}

} // namespace

// Constructor.
template <class LightController>
LightControllerSerializer<LightController>::LightControllerSerializer()
	: ControllerSerializer(LightController::GetComponentType()->Name)
{
}

// Destructor.
template <class LightController>
LightControllerSerializer<LightController>::~LightControllerSerializer()
{
}

// Creates a serializable object from the given parameters.
template <class LightController>
lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> LightControllerSerializer<LightController>::Create(
	const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	SceneParameters sceneParameters = GetSceneParameters(parameters);
	LightControllers<LightController>* lightControllers = GetLightControllers<LightController>(parameters);

	// Light controller
	lean::scoped_ptr<LightController> controller( lightControllers->AddController() );
	controller->SetMaterial( GetLightDefaultMaterial<LightController>(*sceneParameters.ResourceManager, *sceneParameters.Renderer) );

	return controller.transfer();
}

// Loads a mesh controller from the given xml node.
template <class LightController>
lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> LightControllerSerializer<LightController>::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const
{
	SceneParameters sceneParameters = GetSceneParameters(parameters);
	LightControllers<LightController>* lightControllers = GetLightControllers<LightController>(parameters);

	lean::resource_ptr<LightMaterial> lightMaterial;

	utf8_ntr materialName = lean::get_attribute(node, "material");

	if (!materialName.empty())
		lightMaterial = sceneParameters.Renderer->LightMaterials->GetMaterial(
				sceneParameters.ResourceManager->MaterialCache->GetByName(materialName, true)
			);
	else
		lightMaterial = GetLightDefaultMaterial<LightController>(*sceneParameters.ResourceManager, *sceneParameters.Renderer);

	lean::scoped_ptr<LightController> controller( lightControllers->AddController() );
	controller->SetMaterial(lightMaterial);
	ControllerSerializer::Load(controller, node, parameters, queue);

	return controller.transfer();
}

// Saves the given mesh controller to the given XML node.
template <class LightController>
void LightControllerSerializer<LightController>::Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const
{
	ControllerSerializer::Save(pSerializable, node, parameters, queue);

	const LightController &lightController = static_cast<const LightController&>(*pSerializable);
	
	if (const LightMaterial *lightMaterial = lightController.GetMaterial())
	{
		utf8_ntr name = bec::GetCachedName<utf8_ntr>(lightMaterial->GetMaterial());
		if (!name.empty())
			lean::append_attribute( *node.document(), node, "material", name );
		else
			LEAN_LOG_ERROR_MSG("Could not identify LightController material, information will be lost");

		SaveMaterial(lightMaterial->GetMaterial(), parameters, queue);
	}
}

const beEntitySystem::EntityControllerSerializationPlugin<DirectionalLightControllerSerializer> DirectionalLightControllerSerialization;
const beEntitySystem::EntityControllerSerializationPlugin<PointLightControllerSerializer> PointLightControllerSerialization;
const beEntitySystem::EntityControllerSerializationPlugin<SpotLightControllerSerializer> SpotLightControllerSerialization;

} // namespace
