/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beCharacter.h"
#include "bePhysics/PX3/beScene.h"
#include "bePhysics/PX3/beCharacterScene.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beMath.h"

#include "bePhysics/PX3/beRigidActors.h"

#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

// Creates a character.
physx::PxCapsuleController* CreateCharacter(physx::PxControllerManager &manager, physx::PxScene *scene, const physx::PxCapsuleControllerDesc &desc)
{
	physx::PxCapsuleController *pController = static_cast<physx::PxCapsuleController*>(
			manager.createController(scene->getPhysics(), scene, desc)
		);

	if (!pController)
		LEAN_THROW_ERROR_MSG("physx::PxControllerManager::createController()");

	return pController;
}

// Creates a character.
physx::PxCapsuleController* CreateCharacter(physx::PxControllerManager &manager, physx::PxScene *scene,
	const CharacterDesc &desc, const physx::PxMaterial *material, physx::PxUserControllerHitReport *pCallback)
{
	physx::PxCapsuleControllerDesc descPX;
	descPX.radius = desc.Radius;
	descPX.height = desc.Height;
	descPX.slopeLimit = desc.SlopeLimit;
	descPX.stepOffset = desc.StepOffset;
	descPX.contactOffset = desc.SkinWidth;
	descPX.material = const_cast<physx::PxMaterial*>(material);
	descPX.callback = pCallback;

	return CreateCharacter(manager, scene, descPX);
}

// Constructor.
Character::Character(bePhysics::CharacterScene *charScene, bePhysics::Scene *scene,
		const CharacterDesc &desc, const bePhysics::Material *material, CharacterCallback *pCallback)
	: m_pController(
		CreateCharacter(
				**ToImpl(charScene), *ToImpl(scene),
				desc, *ToImpl(material), pCallback
			)
		)
{
}

// Constructor.
Character::Character(physx::PxCapsuleController *pController)
	: m_pController( LEAN_ASSERT_NOT_NULL(pController) )
{
}

// Destructor.
Character::~Character()
{
}

// Moves the character by the given amount.
CharacterCollisionFlags Character::Move(const beMath::fvec3 &move, float minDist)
{
	uint4 collisionFlagsPX = m_pController->move( ToAPI(move), minDist, 0.0f, physx::PxControllerFilters() );

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
bool Character::CheckShape(float radius, float height) const
{
	return false;
}

// Checks if the given shape would currently overlap.
bool Character::CheckShape(const beMath::fvec3 &pos, float radius, float height) const
{
	return false;
}

// Changes the shape.
void Character::SetShape(float radius, float height)
{
	m_pController->setRadius(radius);
	m_pController->setHeight(height);
}

// Gets the current radius.
float Character::GetRadius() const
{
	return m_pController->getRadius();
}

// Gets the current height.
float Character::GetHeight() const
{
	return m_pController->getHeight();
}

// Sets the position.
void Character::SetPosition(const beMath::fvec3 &pos)
{
	m_pController->setPosition( ToXAPI( ToAPI(pos) ) );
}

// Gets the current position.
beMath::fvec3 Character::GetPosition() const
{
	return FromAPI( FromXAPI( m_pController->getPosition() ) );
}

// Gets the current slope limit.
float Character::GetSlopeLimit() const
{
	return m_pController->getSlopeLimit();
}

// Sets the step offset.
void Character::SetStepOffset(float offset)
{
	m_pController->setStepOffset(offset);
}

// Gets the current step offset.
float Character::GetStepOffset() const
{
	return m_pController->getStepOffset();
}

} // namespace

// Creates a character from the given capsule.
lean::resource_ptr<Character, true> CreateCharacter(CharacterScene *charScene, Scene *scene,
	const CharacterDesc &desc, const Material *material, CharacterCallback *pCallback)
{
	return lean::bind_resource<Character>(
			new PX3::Character(charScene, scene, desc, material, pCallback)
		);
}

// Sets filter data for the given character.
void SetSimulationFilterData(Character &character, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	PX3::SetSimulationFilterData( *ToImpl(character)->getActor(), groupFlags, typeFlags, ID); 
}

// Gets filter data from the given character.
void GetSimulationFilterData(const Character &character, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	PX3::GetSimulationFilterData( *ToImpl(character)->getActor(), groupFlags, typeFlags, ID); 
}

} // namespace