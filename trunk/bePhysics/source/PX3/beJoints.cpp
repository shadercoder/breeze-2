/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beJoints.h"
#include <PxExtensionsAPI.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

// Creates a D6 joint.
physx::PxD6Joint* CreateD6Joint(physx::PxPhysics &physics, physx::PxRigidActor *actor1, const physx::PxTransform &frame1,
	physx::PxRigidActor *actor2, const physx::PxTransform &frame2)
{
	physx::PxD6Joint *pJoint = physx::PxD6JointCreate(physics,
		actor1, frame1, actor2, frame2);

	if (!pJoint)
		LEAN_THROW_ERROR_MSG("physx::PxD6JointCreate()");

	return pJoint;
}

} // namespace

} // namespace