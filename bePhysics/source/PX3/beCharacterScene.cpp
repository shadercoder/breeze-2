/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beCharacterScene.h"
#include "bePhysics/PX3/beDevice.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

// Creates a character controller.
physx::PxControllerManager* CreateCharacterScene(physx::PxPhysics *physics)
{
	physx::PxControllerManager *pController = ::PxCreateControllerManager(physics->getFoundation());

	if (!pController)
		LEAN_THROW_ERROR_MSG("PxCreateControllerManager()");

	return pController;
}

// Constructor.
CharacterScene::CharacterScene(bePhysics::Device *device)
	: m_pController( CreateCharacterScene(*ToImpl(device)) )
{
}

// Constructor.
CharacterScene::CharacterScene(physx::PxControllerManager *controller)
	: m_pController( LEAN_ASSERT_NOT_NULL(controller) )
{
}

// Destructor.
CharacterScene::~CharacterScene()
{
}

// Updates all character controllres.
void CharacterScene::Update()
{
}

} // namespace

// Creates a character scene.
lean::resource_ptr<CharacterScene, true> CreateCharacterScene(Device *device)
{
	return lean::bind_resource<CharacterScene>(
			new PX3::CharacterScene(device)
		);
}

} // namespace