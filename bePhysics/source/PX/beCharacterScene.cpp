/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beCharacterScene.h"
#include "bePhysics/PX/beDevice.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

// Creates a character controller.
physx::PxControllerManager* CreateCharacterScene(physx::PxPhysics *pPhysics)
{
	physx::PxControllerManager *pController = ::PxCreateControllerManager(pPhysics->getFoundation());

	if (!pController)
		LEAN_THROW_ERROR_MSG("PxCreateControllerManager()");

	return pController;
}

// Constructor.
CharacterScenePX::CharacterScenePX(Device &device)
	: m_pController( CreateCharacterScene(ToImpl(device).Get()) )
{
}

// Constructor.
CharacterScenePX::CharacterScenePX(physx::PxControllerManager *pController)
	: m_pController( LEAN_ASSERT_NOT_NULL(pController) )
{
}

// Destructor.
CharacterScenePX::~CharacterScenePX()
{
}

// Updates all character controllres.
void CharacterScenePX::Update()
{
}

} // namespace