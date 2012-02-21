/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beEntityController.h"
#include "bePhysics/beSceneController.h"

namespace bePhysics
{

// Constructor.
EntityController::EntityController(beEntitySystem::Entity *pEntity, SceneController *pScene)
	: beEntitySystem::EntityController(pEntity),
	m_pScene( LEAN_ASSERT_NOT_NULL(pScene) )
{
}

// Destructor.
EntityController::~EntityController()
{
}

} // namespace