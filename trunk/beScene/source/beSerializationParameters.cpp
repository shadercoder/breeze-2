/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beSerializationParameters.h"

#include <beEntitySystem/beSerializationParameters.h>

namespace beScene
{

// Gets the serialization parameter IDs.
const SceneParameterIDs& GetSceneParameterIDs()
{
	beCore::ParameterLayout &layout = beEntitySystem::GetSerializationParameters();

	static SceneParameterIDs parameterIDs(
			layout.Add("beScene.ResourceManager"),
			layout.Add("beScene.Renderer"),
			layout.Add("beScene.SceneController"),
			layout.Add("beScene.Scenery")
		);

	return parameterIDs;
}

// Sets the given scene parameters in the given parameter set.
void SetSceneParameters(beCore::ParameterSet &parameters, const SceneParameters &sceneParameters)
{
	const beCore::ParameterLayout &layout = beEntitySystem::GetSerializationParameters();
	const SceneParameterIDs& parameterIDs = GetSceneParameterIDs();

	parameters.SetValue(layout, parameterIDs.ResourceManager, sceneParameters.ResourceManager);
	parameters.SetValue(layout, parameterIDs.Renderer, sceneParameters.Renderer);
	parameters.SetValue(layout, parameterIDs.SceneController, sceneParameters.SceneController);
	parameters.SetValue(layout, parameterIDs.Scenery, sceneParameters.Scenery);
}

// Sets the given scene parameters in the given parameter set.
SceneParameters GetSceneParameters(const beCore::ParameterSet &parameters)
{
	SceneParameters sceneParameters;

	const beCore::ParameterLayout &layout = beEntitySystem::GetSerializationParameters();
	const SceneParameterIDs& parameterIDs = GetSceneParameterIDs();

	sceneParameters.ResourceManager = parameters.GetValueChecked< beScene::ResourceManager* >(layout, parameterIDs.ResourceManager);
	sceneParameters.Renderer = parameters.GetValueChecked< EffectDrivenRenderer* >(layout, parameterIDs.Renderer);
	sceneParameters.SceneController = parameters.GetValueChecked< SceneController* >(layout, parameterIDs.SceneController);
	sceneParameters.Scenery = parameters.GetValueChecked< DynamicScenery* >(layout, parameterIDs.Scenery);

	return sceneParameters;
}

} // namespace
