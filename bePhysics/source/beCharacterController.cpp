/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beCharacterController.h"
#include <beEntitySystem/beEntities.h>
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

BE_CORE_PUBLISH_COMPONENT(CharacterController)

// Constructor.
CharacterController::CharacterController(CharacterSceneController *pScene, Character *pActor, const Material *pMaterial)
	: m_pScene( LEAN_ASSERT_NOT_NULL(pScene) ),
	
	m_pActor( LEAN_ASSERT_NOT_NULL(pActor) ),
	m_pMaterial( pMaterial ),
	m_bDynamic(false),

	m_pEntity(nullptr),
	m_lastScaling(1.0f)
{
//	Scale(*ToImpl(*m_pActor)->getActor(), physx::PxVec3(0.9f));
}

// Destructor.
CharacterController::~CharacterController()
{
	// WARNING: Never forget, will crash otherwise
	Detach(nullptr);
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
void CharacterController::Synchronize(beEntitySystem::EntityHandle entity)
{
	using bees::Entities;

	const Entities::Transformation &trafo = Entities::GetTransformation(entity);
	ToImpl(*m_pActor)->setPosition( PX3::ToXAPI(PX3::ToAPI(trafo.Position)) );

	if (m_lastScaling != trafo.Scaling)
	{
		physx::PxShape *pShape = nullptr;
		ToImpl(*m_pActor)->getActor()->getShapes(&pShape, 1);

		physx::PxCapsuleGeometry geom;
		pShape->getCapsuleGeometry(geom);

		PX3::Scale( geom, pShape->getLocalPose(), PX3::ToAPI(trafo.Scaling / m_lastScaling) );
		
		ToImpl(*m_pActor)->setRadius(geom.radius);
		ToImpl(*m_pActor)->setHeight(2.0f * geom.halfHeight);
		
		m_lastScaling = trafo.Scaling;
		// Update inertia
		SetMassAndInertia(GetMass());
	}
}

// Synchronizes this controller with the simulation.
void CharacterController::Flush(const beEntitySystem::EntityHandle entity)
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
		bees::Entity *pWasAttachedTo = m_pEntity;

		if (pWasAttachedTo)
			Detach(pWasAttachedTo);

		ToImpl(*m_pActor)->getActor()->setRigidDynamicFlags((bDynamic) ? physx::PxRigidDynamicFlags() : physx::PxRigidDynamicFlag::eKINEMATIC);
		m_bDynamic = bDynamic;

		if (pWasAttachedTo)
			Attach(pWasAttachedTo);
	}
}

// Attaches this controller to the scene.
void CharacterController::Attach(beEntitySystem::Entity *entity)
{
	if (m_pEntity)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_pEntity = entity;

	Synchronize(m_pEntity->Handle());

	m_pScene->AddSynchronized(this, beEntitySystem::SynchronizedFlags::Fetch);
}

// Detaches this controller from the scene.
void CharacterController::Detach(beEntitySystem::Entity *entity)
{
	if (m_pEntity != entity || m_pEntity && !entity)
		return;

	m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_pEntity = nullptr;
}

} // namespace