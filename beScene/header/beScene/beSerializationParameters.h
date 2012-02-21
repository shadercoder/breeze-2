/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_SERIALIZATION_PARAMETERS
#define BE_SCENE_SERIALIZATION_PARAMETERS

#include "beScene.h"

#include <beCore/beParameterSet.h>

namespace beScene
{

// Prototypes
class ResourceManager;
class EffectDrivenRenderer;
class SceneController;
class DynamicScenery;

/// Scene parameter IDs.
struct SceneParameterIDs
{
	uint4 ResourceManager;
	uint4 Renderer;
	uint4 SceneController;
	uint4 Scenery;

	/// Non-initializing constructor.
	SceneParameterIDs() { }
	/// Constructor.
	SceneParameterIDs(uint4 resourceManagerID, uint4 rendererID,
		uint4 sceneControllerID, uint4 sceneryID)
			: ResourceManager(resourceManagerID),
			Renderer(rendererID),
			SceneController(sceneControllerID),
			Scenery(sceneryID) { }
};

/// Scene parameters.
struct SceneParameters
{
	ResourceManager *ResourceManager;
	EffectDrivenRenderer *Renderer;
	SceneController *SceneController;
	DynamicScenery *Scenery;

	/// Default constructor.
	SceneParameters()
		: ResourceManager(),
		Renderer(),
		SceneController(),
		Scenery() { }
	/// Constructor.
	SceneParameters(class ResourceManager *pResourceManager,
		EffectDrivenRenderer *pRenderer,
		class SceneController *pSceneController = nullptr,
		class DynamicScenery *pScenery = nullptr)
			: ResourceManager(pResourceManager),
			Renderer(pRenderer),
			SceneController(pSceneController),
			Scenery(pScenery) { }
};

/// Gets the serialization parameter IDs.
BE_SCENE_API const SceneParameterIDs& GetSceneParameterIDs();

/// Sets the given scene parameters in the given parameter set.
BE_SCENE_API void SetSceneParameters(beCore::ParameterSet &parameters, const SceneParameters &sceneParameters);
/// Sets the given scene parameters in the given parameter set.
BE_SCENE_API SceneParameters GetSceneParameters(const beCore::ParameterSet &parameters);

} // namespace

#endif