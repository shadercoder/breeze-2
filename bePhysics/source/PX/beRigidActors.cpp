/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beRigidActors.h"
#include "bePhysics/PX/beShapes.h"
#include "bePhysics/PX/beDevice.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

// Creates a dynamic rigid body.
physx::PxRigidDynamic* CreateRigidDynamic(physx::PxPhysics *pPhysics)
{
	physx::PxRigidDynamic *pRigid = pPhysics->createRigidDynamic(physx::PxTransform::createIdentity());

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createRigidDynamic()");

	return pRigid;
}

// Constructor.
RigidDynamicPX::RigidDynamicPX(Device &device)
	: m_pActor( CreateRigidDynamic(ToImpl(device)) )
{
}

// Constructor.
RigidDynamicPX::RigidDynamicPX(physx::PxRigidDynamic *pActor)
	: m_pActor( LEAN_ASSERT_NOT_NULL(pActor) )
{
}

// Destructor.
RigidDynamicPX::~RigidDynamicPX()
{
}

// Creates a static rigid actor.
physx::PxRigidStatic* CreateRigidStatic(physx::PxPhysics *pPhysics)
{
	physx::PxRigidStatic *pRigid = pPhysics->createRigidStatic(physx::PxTransform::createIdentity());

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createRigidStatic()");

	return pRigid;
}

// Constructor.
RigidStaticPX::RigidStaticPX(Device &device)
	: m_pActor( CreateRigidStatic(ToImpl(device)) )
{
}

// Constructor.
RigidStaticPX::RigidStaticPX(physx::PxRigidStatic *pActor)
	: m_pActor( LEAN_ASSERT_NOT_NULL(pActor) )
{
}

// Destructor.
RigidStaticPX::~RigidStaticPX()
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