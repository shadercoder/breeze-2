/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beEntityController.h"
#include "beScene/beSceneController.h"

namespace beScene
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