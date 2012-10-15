/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_PHYSICS_SERIALIZATION_PARAMETERS
#define BE_PHYSICS_SERIALIZATION_PARAMETERS

#include "bePhysics.h"

#include <beCore/beParameterSet.h>

namespace bePhysics
{

// Prototypes
class Device;
class ResourceManager;
class SceneController;
class Scene;

/// Physics parameter IDs.
struct PhysicsParameterIDs
{
	uint4 Device;
	uint4 ResourceManager;
	uint4 SceneController;
	uint4 Scene;

	/// Non-initializing constructor.
	PhysicsParameterIDs() { }
	/// Constructor.
	PhysicsParameterIDs(uint4 deviceID, uint4 resourceManagerID, uint4 sceneControllerID, uint4 sceneID)
			: Device(deviceID),
			ResourceManager(resourceManagerID),
			SceneController(sceneControllerID),
			Scene(sceneID) { }
};

/// Physics parameters.
struct PhysicsParameters
{
	Device *Device;
	ResourceManager *ResourceManager;
	SceneController *SceneController;
	Scene *Scene;

	/// Default constructor.
	PhysicsParameters()
		: Device(),
		ResourceManager(),
		SceneController(),
		Scene() { }
	/// Constructor.
	PhysicsParameters(class Device *pDevice,
		class ResourceManager *pResourceManager,
		class SceneController *pSceneController = nullptr,
		class Scene *pScene = nullptr)
			: Device(pDevice),
			ResourceManager(pResourceManager),
			SceneController(pSceneController),
			Scene(pScene) { }
};

/// Gets the serialization parameter IDs.
BE_PHYSICS_API const PhysicsParameterIDs& GetPhysicsParameterIDs();

/// Sets the given scene parameters in the given parameter set.
BE_PHYSICS_API void SetPhysicsParameters(beCore::ParameterSet &parameters, const PhysicsParameters &sceneParameters);
/// Sets the given scene parameters in the given parameter set.
BE_PHYSICS_API PhysicsParameters GetPhysicsParameters(const beCore::ParameterSet &parameters);

} // namespace

#endif