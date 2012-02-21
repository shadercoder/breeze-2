/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidDynamicController.h"
#include <beEntitySystem/beEntity.h>
#include "bePhysics/beSceneController.h"
#include "bePhysics/PX/beScene.h"
#include "bePhysics/PX/beRigidActors.h"
#include "bePhysics/PX/beMaterial.h"
#include "bePhysics/PX/beShapes.h"
#include "bePhysics/PX/beMath.h"
#include <PxExtensionsAPI.h>

namespace bePhysics
{

// Constructor.
RigidDynamicController::RigidDynamicController(beEntitySystem::Entity *pEntity, SceneController *pScene, RigidDynamic *pActor,
		const ShapeCompound *pShape, const Material *pMaterial)
	: EntityController(pEntity, pScene),
	m_bAttached(false),
	
	m_pActor( LEAN_ASSERT_NOT_NULL(pActor) ),
	m_pShape( pShape ),
	m_pMaterial( pMaterial ),
	m_bKinematic(false),

	m_lastScaling(1.0f)
{
	Synchronize();
}

// Destructor.
RigidDynamicController::~RigidDynamicController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Sets the material.
void RigidDynamicController::SetMaterial(const Material *pMaterial)
{
	if (pMaterial && pMaterial != m_pMaterial)
	{
		const uint4 shapeCount = ToImpl(*m_pActor)->getNbShapes();

		std::vector<physx::PxShape*> shapes(shapeCount);
		ToImpl(*m_pActor)->getShapes(&shapes[0], shapeCount);

		for (size_t i = 0; i < shapeCount; ++i)
			shapes[i]->setMaterials(
					&const_cast<physx::PxMaterial *const &>( ToImpl(*pMaterial).Get() ),
					1
				);

		m_pMaterial = pMaterial;
	}
}

// Sets the velocity.
void RigidDynamicController::SetVelocity(const beMath::fvec3 &v)
{
	ToImpl(*m_pActor)->setLinearVelocity( ToImpl(v) );
}

// Gets the velocity.
beMath::fvec3 RigidDynamicController::GetVelocity() const
{
	return ToBE( ToImpl(*m_pActor)->getLinearVelocity() );
}

// Sets the mass.
void RigidDynamicController::SetMass(float mass)
{
	ToImpl(*m_pActor)->setMass( mass );
}

// Sets the mass.
void RigidDynamicController::SetMassAndInertia(float mass)
{
	physx::PxRigidBodyExt::setMassAndUpdateInertia( *ToImpl(*m_pActor), mass );
}

// Sets the mass.
float RigidDynamicController::GetMass() const
{
	return ToImpl(*m_pActor)->getMass();
}

// Forces this controller into synchronization with the simulation.
void RigidDynamicController::Synchronize()
{
	ToImpl(*m_pActor)->setGlobalPose( ToTransform(m_pEntity->GetOrientation(), m_pEntity->GetPosition()) );

	if (m_lastScaling != m_pEntity->GetScaling())
	{
		Scale( *ToImpl(*m_pActor), ToImpl(m_pEntity->GetScaling() / m_lastScaling) );
		m_lastScaling = m_pEntity->GetScaling();
		// Update inertia
		SetMassAndInertia(GetMass());
	}
}

// Synchronizes this controller with the simulation.
void RigidDynamicController::Flush()
{
	ToImpl(*m_pActor)->setKinematicTarget( ToTransform(m_pEntity->GetOrientation(), m_pEntity->GetPosition()) );
}

// Synchronizes the simulation with this controller.
void RigidDynamicController::Fetch()
{
	physx::PxTransform pose = ToImpl(*m_pActor)->getGlobalPose();
	m_pEntity->SetPosition( ToBE(pose.p) );
	m_pEntity->SetOrientation( ToBE(physx::PxMat33(pose.q)) );
}

// Converts the controlled actor between non-kinematic and kinematic.
void RigidDynamicController::SetKinematic(bool bKinematic)
{
	if (bKinematic != m_bKinematic)
	{
		bool bWasAttached = m_bAttached;

		if (bWasAttached)
			Detach();

		ToImpl(*m_pActor)->setRigidDynamicFlags((bKinematic) ? physx::PxRigidDynamicFlag::eKINEMATIC : physx::PxRigidDynamicFlags());
		m_bKinematic = bKinematic;

		if (bWasAttached)
			Attach();
	}
}

// Attaches this controller to the scene.
void RigidDynamicController::Attach()
{
	if (m_bAttached)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_bAttached = true;

	Synchronize();

	ToImpl(m_pScene->GetScene())->addActor(*ToImpl(*m_pActor));
	m_pScene->AddSynchronized(
		this,
		(m_bKinematic) ? beEntitySystem::SynchronizedFlags::Flush : beEntitySystem::SynchronizedFlags::Fetch);
}

// Detaches this controller from the scene.
void RigidDynamicController::Detach()
{
	if (!m_bAttached)
		return;

	ToImpl(m_pScene->GetScene())->removeActor(*ToImpl(*m_pActor));
	m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bAttached = false;
}

// Gets the controller type.
utf8_ntr RigidDynamicController::GetControllerType()
{
	return utf8_ntr("RigidDynamicController");
}

} // namespace