/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beLightControllerSerializer.h"
#include "beScene/beDirectionalLightController.h"
#include "beScene/bePointLightController.h"
#include "beScene/beSpotLightController.h"

#include <beEntitySystem/beControllerSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include "beScene/beSerializationParameters.h"
#include <beCore/beParameterSet.h>

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"

namespace beScene
{

namespace
{

/// Gets a default material for the given light type.
template <class ControllerType>
lean::resource_ptr<ControllerType, true> CreateDefaultController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer);

template<>
lean::resource_ptr<PointLightController, true> CreateDefaultController<PointLightController>(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return CreateDefaultPointLightController(pEntity, pScene, pScenery, resources, renderer);
}

template<>
lean::resource_ptr<SpotLightController, true> CreateDefaultController<SpotLightController>(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return CreateDefaultSpotLightController(pEntity, pScene, pScenery, resources, renderer);
}

template<>
lean::resource_ptr<DirectionalLightController, true> CreateDefaultController<DirectionalLightController>(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery,
	ResourceManager &resources, EffectDrivenRenderer &renderer)
{
	return CreateDefaultDirectionalLightController(pEntity, pScene, pScenery, resources, renderer);
}

} // namespace

// Constructor.
template <class LightController>
LightControllerSerializer<LightController>::LightControllerSerializer()
	: ControllerSerializer(LightController::GetControllerType())
{
}

// Destructor.
template <class LightController>
LightControllerSerializer<LightController>::~LightControllerSerializer()
{
}

// Creates a serializable object from the given parameters.
template <class LightController>
lean::resource_ptr<beEntitySystem::Controller, true> LightControllerSerializer<LightController>::Create(
	const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	SceneParameters sceneParameters = GetSceneParameters(parameters);

	// Light controller
	lean::resource_ptr<LightController> pController = CreateDefaultController<LightController>(
			entityParameters.Entity, sceneParameters.SceneController, sceneParameters.Scenery,
			*sceneParameters.ResourceManager, *sceneParameters.Renderer
		);

	return pController.transfer();
}

// Loads a mesh controller from the given xml node.
template <class LightController>
lean::resource_ptr<beEntitySystem::Controller, true> LightControllerSerializer<LightController>::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::LoadJob> &queue) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	SceneParameters sceneParameters = GetSceneParameters(parameters);

	// TODO: Don't create default?
	lean::resource_ptr<LightController> pController = CreateDefaultController<LightController>(
			entityParameters.Entity, sceneParameters.SceneController, sceneParameters.Scenery,
			*sceneParameters.ResourceManager, *sceneParameters.Renderer
		);
	
	ControllerSerializer::Load(pController, node, parameters, queue);

	return pController.transfer();
}

// Saves the given mesh controller to the given XML node.
template <class LightController>
void LightControllerSerializer<LightController>::Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::SaveJob> &queue) const
{
	ControllerSerializer::Save(pSerializable, node, parameters, queue);
}

const beEntitySystem::ControllerSerializationPlugin<DirectionalLightControllerSerializer> DirectionalLightControllerSerialization;
const beEntitySystem::ControllerSerializationPlugin<PointLightControllerSerializer> PointLightControllerSerialization;
const beEntitySystem::ControllerSerializationPlugin<SpotLightControllerSerializer> SpotLightControllerSerialization;

} // namespace
