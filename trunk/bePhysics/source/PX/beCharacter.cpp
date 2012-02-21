/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beCharacter.h"
#include "bePhysics/PX/beScene.h"
#include "bePhysics/PX/beCharacterScene.h"
#include "bePhysics/PX/beMaterial.h"
#include "bePhysics/PX/beMath.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

// Creates a character.
physx::PxCapsuleController* CreateCharacter(physx::PxControllerManager &manager, physx::PxScene *pScene, const physx::PxCapsuleControllerDesc &desc)
{
	physx::PxCapsuleController *pController = static_cast<physx::PxCapsuleController*>(
		manager.createController(pScene->getPhysics(), pScene, desc) );

	if (!pController)
		LEAN_THROW_ERROR_MSG("physx::PxControllerManager::createController()");

	return pController;
}

// Creates a character.
physx::PxCapsuleController* CreateCharacter(physx::PxControllerManager &manager, physx::PxScene *pScene,
	const CharacterDesc &desc, const physx::PxMaterial *pMaterial, physx::PxUserControllerHitReport *pCallback)
{
	physx::PxCapsuleControllerDesc descPX;
	descPX.radius = desc.Radius;
	descPX.height = desc.Height;
	descPX.slopeLimit = desc.SlopeLimit;
	descPX.stepOffset = desc.StepOffset;
	descPX.contactOffset = desc.SkinWidth;
	descPX.material = const_cast<physx::PxMaterial*>(pMaterial);
	descPX.callback = pCallback;

	return CreateCharacter(manager, pScene, descPX);
}

// Constructor.
CharacterPX::CharacterPX(CharacterScene &charScene, Scene &scene, const CharacterDesc &desc, const Material &material, CharacterCallback *pCallback)
	: m_pController( CreateCharacter(*ToImpl(charScene), ToImpl(scene), desc, ToImpl(material), pCallback) )
{
}

// Constructor.
CharacterPX::CharacterPX(physx::PxCapsuleController *pController)
	: m_pController( LEAN_ASSERT_NOT_NULL(pController) )
{
}

// Destructor.
CharacterPX::~CharacterPX()
{
}

// Moves the character by the given amount.
CharacterCollisionFlags CharacterPX::Move(const beMath::fvec3 &move, float minDist)
{
	uint4 collisionFlagsPX = m_pController->move( ToImpl(move), minDist, 0.0f, physx::PxControllerFilters() );

	CharacterCollisionFlags collisionFlags = 0;

	if (collisionFlagsPX & physx::PxControllerFlag::eCOLLISION_DOWN)
		collisionFlags |= CharacterCollision::Down;
	if (collisionFlagsPX & physx::PxControllerFlag::eCOLLISION_SIDES)
		collisionFlags |= CharacterCollision::Sides;
	if (collisionFlagsPX & physx::PxControllerFlag::eCOLLISION_UP)
		collisionFlags |= CharacterCollision::Up;

	return collisionFlags;
}

// Checks if the given shape would currently overlap.
bool CharacterPX::CheckShape(float radius, float height) const
{
	return false;
}

// Checks if the given shape would currently overlap.
bool CharacterPX::CheckShape(const beMath::fvec3 &pos, float radius, float height) const
{
	return false;
}

// Changes the shape.
void CharacterPX::SetShape(float radius, float height)
{
	m_pController->setRadius(radius);
	m_pController->setHeight(height);
}

// Gets the current radius.
float CharacterPX::GetRadius() const
{
	return m_pController->getRadius();
}

// Gets the current height.
float CharacterPX::GetHeight() const
{
	return m_pController->getHeight();
}

// Sets the position.
void CharacterPX::SetPosition(const beMath::fvec3 &pos)
{
	m_pController->setPosition( ToExPX( ToImpl(pos) ) );
}

// Gets the current position.
beMath::fvec3 CharacterPX::GetPosition() const
{
	return ToBE( ToPX( m_pController->getPosition() ) );
}

// Gets the current slope limit.
float CharacterPX::GetSlopeLimit() const
{
	return m_pController->getSlopeLimit();
}

// Sets the step offset.
void CharacterPX::SetStepOffset(float offset)
{
	m_pController->setStepOffset(offset);
}

// Gets the current step offset.
float CharacterPX::GetStepOffset() const
{
	return m_pController->getStepOffset();
}

} // namespace