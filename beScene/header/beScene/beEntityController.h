/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_ENTITYCONTROLLER
#define BE_SCENE_ENTITYCONTROLLER

#include "beScene.h"
#include <beEntitySystem/beEntityController.h>
#include <lean/smart/resource_ptr.h>

namespace beScene
{

// Prototypes
class SceneController;

/// Scene entity controller base class.
class LEAN_INTERFACE EntityController : public beEntitySystem::EntityController
{
protected:
	/// Scene.
	const lean::resource_ptr<SceneController> m_pScene;

public:
	/// Constructor.
	BE_SCENE_API EntityController(beEntitySystem::Entity *pEntity, SceneController *pScene);
	/// Destructor.
	BE_SCENE_API ~EntityController();

	/// Gets the scene controller.
	LEAN_INLINE SceneController* GetScene() const { return m_pScene; }
};

} // namespace

#endif