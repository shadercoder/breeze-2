/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beCharacterController.h"
#include <beEntitySystem/beEntity.h>
#include "bePhysics/beCharacterSceneController.h"
#include "bePhysics/PX3/beCharacter.h"
#include "bePhysics/PX3/beScene.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beMath.h"
#include <PxExtensionsAPI.h>

namespace bePhysics
{

// Constructor.
CharacterController::CharacterController(beEntitySystem::Entity *pEntity, CharacterSceneController *pScene, Character *pActor, const Material *pMaterial)
	: EntityController(pEntity),
	m_pScene(pScene),
	m_bAttached(false),
	
	m_pActor( LEAN_ASSERT_NOT_NULL(pActor) ),
	m_pMaterial( pMaterial ),
	m_bDynamic(false),

	m_lastScaling(1.0f)
{
//	Scale(*ToImpl(*m_pActor)->getActor(), physx::PxVec3(0.9f));

	Synchronize();
}

// Destructor.
CharacterController::~CharacterController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Sets the material.
void CharacterController::SetMaterial(const Material *pMaterial)
{
	if (pMaterial && pMaterial != m_pMaterial)
	{
		physx::PxRigidActor *pActor = ToImpl(*m_pActor)->getActor();
		const uint4 shapeCount = pActor->getNbShapes();

		std::vector<physx::PxShape*> shapes(shapeCount);
		pActor->getShapes(&shapes[0], shapeCount);

		for (size_t i = 0; i < shapeCount; ++i)
			shapes[i]->setMaterials(
					&const_cast<physx::PxMaterial *const &>( ToImpl(*pMaterial).Get() ),
					1
				);

		m_pMaterial = pMaterial;
	}
}

// Sets the mass.
void CharacterController::SetMass(float mass)
{
	ToImpl(*m_pActor)->getActor()->setMass( mass );
}

// Sets the mass.
void CharacterController::SetMassAndInertia(float mass)
{
	physx::PxRigidBodyExt::setMassAndUpdateInertia( *ToImpl(*m_pActor)->getActor(), mass );
}

// Sets the mass.
float CharacterController::GetMass() const
{
	return ToImpl(*m_pActor)->getActor()->getMass();
}

// Forces this controller into synchronization with the simulation.
void CharacterController::Synchronize()
{
	ToImpl(*m_pActor)->setPosition( PX3::ToXAPI(PX3::ToAPI(m_pEntity->GetPosition())) );

	if (m_lastScaling != m_pEntity->GetScaling())
	{
		physx::PxShape *pShape = nullptr;
		ToImpl(*m_pActor)->getActor()->getShapes(&pShape, 1);

		physx::PxCapsuleGeometry geom;
		pShape->getCapsuleGeometry(geom);

		PX3::Scale( geom, pShape->getLocalPose(), PX3::ToAPI(m_pEntity->GetScaling() / m_lastScaling) );
		
		ToImpl(*m_pActor)->setRadius(geom.radius);
		ToImpl(*m_pActor)->setHeight(2.0f * geom.halfHeight);
		
		m_lastScaling = m_pEntity->GetScaling();
		// Update inertia
		SetMassAndInertia(GetMass());
	}
}

// Synchronizes this controller with the simulation.
void CharacterController::Flush()
{
}

// Synchronizes the simulation with this controller.
void CharacterController::Fetch()
{
	physx::PxExtendedVec3 pos = ToImpl(*m_pActor)->getPosition();
	m_pEntity->SetPosition( PX3::FromAPI( PX3::FromXAPI(pos) ) );
}

// Converts the controlled actor between non-kinematic and kinematic.
void CharacterController::SetDynamic(bool bDynamic)
{
	if (bDynamic != m_bDynamic)
	{
		bool bWasAttached = m_bAttached;

		if (bWasAttached)
			Detach();

		ToImpl(*m_pActor)->getActor()->setRigidDynamicFlags((bDynamic) ? physx::PxRigidDynamicFlags() : physx::PxRigidDynamicFlag::eKINEMATIC);
		m_bDynamic = bDynamic;

		if (bWasAttached)
			Attach();
	}
}

// Attaches this controller to the scene.
void CharacterController::Attach()
{
	if (m_bAttached)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_bAttached = true;

	Synchronize();

	m_pScene->AddSynchronized(this, beEntitySystem::SynchronizedFlags::Fetch);
}

// Detaches this controller from the scene.
void CharacterController::Detach()
{
	if (!m_bAttached)
		return;

	m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bAttached = false;
}

// Gets the controller type.
utf8_ntr CharacterController::GetControllerType()
{
	return utf8_ntr("CharacterController");
}

} // namespace