/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_ENTITYCONTROLLER
#define BE_PHYSICS_ENTITYCONTROLLER

#include "bePhysics.h"
#include <beEntitySystem/beEntityController.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

// Prototypes
class SceneController;

/// Scene entity controller base class.
class EntityController : public beEntitySystem::EntityController
{
protected:
	/// Scene.
	const lean::resource_ptr<SceneController> m_pScene;

public:
	/// Constructor.
	BE_PHYSICS_API EntityController(beEntitySystem::Entity *pEntity, SceneController *pScene);
	/// Destructor.
	BE_PHYSICS_API ~EntityController();

	/// Gets the scene controller.
	LEAN_INLINE SceneController* GetScene() const { return m_pScene; }
};

} // namespace

#endif