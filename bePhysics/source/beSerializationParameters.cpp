/******************************************************/
/* breeze Engine Physics Module  (c) Tobias Zirr 2011 */
/******************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beSerializationParameters.h"

#include <beEntitySystem/beSerializationParameters.h>

namespace bePhysics
{

// Gets the serialization parameter IDs.
const PhysicsParameterIDs& GetPhysicsParameterIDs()
{
	beCore::ParameterLayout &layout = beEntitySystem::GetSerializationParameters();

	static PhysicsParameterIDs parameterIDs(
			layout.Add("bePhysics.Device"),
			layout.Add("bePhysics.ResourceManager"),
			layout.Add("bePhysics.SceneController"),
			layout.Add("bePhysics.Scene")
		);

	return parameterIDs;
}

// Sets the given scene parameters in the given parameter set.
void SetPhysicsParameters(beCore::ParameterSet &parameters, const PhysicsParameters &physicsParameters)
{
	const beCore::ParameterLayout &layout = beEntitySystem::GetSerializationParameters();
	const PhysicsParameterIDs& parameterIDs = GetPhysicsParameterIDs();

	parameters.SetValue(layout, parameterIDs.Device, physicsParameters.Device);
	parameters.SetValue(layout, parameterIDs.ResourceManager, physicsParameters.ResourceManager);
	parameters.SetValue(layout, parameterIDs.SceneController, physicsParameters.SceneController);
	parameters.SetValue(layout, parameterIDs.Scene, physicsParameters.Scene);
}

// Sets the given scene parameters in the given parameter set.
PhysicsParameters GetPhysicsParameters(const beCore::ParameterSet &parameters)
{
	PhysicsParameters physicsParameters;

	const beCore::ParameterLayout &layout = beEntitySystem::GetSerializationParameters();
	const PhysicsParameterIDs& parameterIDs = GetPhysicsParameterIDs();

	physicsParameters.Device = parameters.GetValueChecked< Device* >(layout, parameterIDs.Device);
	physicsParameters.ResourceManager = parameters.GetValueChecked< ResourceManager* >(layout, parameterIDs.ResourceManager);
	physicsParameters.SceneController = parameters.GetValueChecked< SceneController* >(layout, parameterIDs.SceneController);
	physicsParameters.Scene = parameters.GetValueChecked< Scene* >(layout, parameterIDs.Scene);

	return physicsParameters;
}

} // namespace
