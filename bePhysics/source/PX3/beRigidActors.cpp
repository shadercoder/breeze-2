/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beDevice.h"
#include <PxExtensionsAPI.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

// Creates a dynamic rigid body.
physx::PxRigidDynamic* CreateRigidDynamic(physx::PxPhysics &physics)
{
	physx::PxRigidDynamic *pRigid = physics.createRigidDynamic(physx::PxTransform::createIdentity());

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createRigidDynamic()");

	return pRigid;
}

// Adds the given shape.
void AddShape(physx::PxRigidActor &actor, const physx::PxGeometry &shape, const physx::PxMaterial *material, const physx::PxTransform &transform)
{
	if (!actor.createShape(shape, *material, transform))
		LEAN_THROW_ERROR_MSG("PxRigidDynamic::createShape()");
}

// Sets the given mass.
void SetMass(physx::PxRigidBody &actor, float mass)
{
	if (!physx::PxRigidBodyExt::setMassAndUpdateInertia(actor, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");
}

// Sets the mass from the given density.
void SetDensity(physx::PxRigidBody &actor, float density)
{
	if (!physx::PxRigidBodyExt::updateMassAndInertia(actor, density))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::updateMassAndInertia()");
}

// Constructor.
RigidDynamic::RigidDynamic(bePhysics::Device &device)
	: m_pActor( CreateRigidDynamic(*ToImpl(device)) )
{
}

// Constructor.
RigidDynamic::RigidDynamic(physx::PxRigidDynamic *actor)
	: m_pActor( LEAN_ASSERT_NOT_NULL(actor) )
{
}

// Destructor.
RigidDynamic::~RigidDynamic()
{
}

// Creates a static rigid actor.
physx::PxRigidStatic* CreateRigidStatic(physx::PxPhysics &physics)
{
	physx::PxRigidStatic *pRigid = physics.createRigidStatic(physx::PxTransform::createIdentity());

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createRigidStatic()");

	return pRigid;
}

// Constructor.
RigidStatic::RigidStatic(bePhysics::Device &device)
	: m_pActor( CreateRigidStatic(*ToImpl(device)) )
{
}

// Constructor.
RigidStatic::RigidStatic(physx::PxRigidStatic *actor)
	: m_pActor( LEAN_ASSERT_NOT_NULL(actor) )
{
}

// Destructor.
RigidStatic::~RigidStatic()
{
}

// Scales the given rigid actor as precisely as possible.
void Scale(physx::PxRigidActor &actor, const physx::PxVec3 &scaling)
{
	const uint4 shapeCount = actor.getNbShapes();

	for (uint4 i = 0; i < shapeCount; ++i)
	{
		physx::PxShape *pShape = nullptr;
		actor.getShapes(&pShape, 1, i);

		Scale( *LEAN_ASSERT_NOT_NULL(pShape), scaling );
	}
}

// Sets filter data for the given actor.
void SetSimulationFilterData(physx::PxRigidActor &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	const uint4 shapeCount = actor.getNbShapes();

	for (uint4 i = 0; i < shapeCount; ++i)
	{
		physx::PxShape *pShape = nullptr;
		actor.getShapes(&pShape, 1, i);
		
		physx::PxFilterData filterData = pShape->getSimulationFilterData();
		ToFilterData(filterData, groupFlags, typeFlags, ID);
		pShape->setSimulationFilterData(filterData);
	}
}

// Gets filter data from the given actor.
void GetSimulationFilterData(const physx::PxRigidActor &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	physx::PxShape *pShape = nullptr;
	actor.getShapes(&pShape, 1);

	if (pShape)
	{
		physx::PxFilterData filterData = pShape->getSimulationFilterData();
		groupFlags = GetGroupFlags(filterData);
		typeFlags = GetTypeFlags(filterData);
		ID = GetID(filterData);
	}
}

} // namespace

// Sets filter data for the given actor.
void SetSimulationFilterData(RigidDynamic &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	PX3::SetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

// Sets filter data for the given actor.
void SetSimulationFilterData(RigidStatic &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	PX3::SetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

// Gets filter data from the given actor.
void GetSimulationFilterData(const RigidDynamic &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	PX3::GetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

// Gets filter data from the given actor.
void GetSimulationFilterData(const RigidStatic &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	PX3::GetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

} // namespace