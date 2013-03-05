/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_JOINTS_PX
#define BE_PHYSICS_JOINTS_PX

#include "bePhysics.h"
#include "../beRigidActors.h"
#include <beCore/beWrapper.h>
#include <lean/tags/noncopyable.h>
#include "beAPI.h"
#include <PxExtensionsAPI.h>

namespace bePhysics
{

// Prototypes
class Device;

namespace PX3
{

/// Creates a D6 joint.
BE_PHYSICS_PX_API physx::PxD6Joint* CreateD6Joint(physx::PxPhysics &physics, physx::PxRigidActor *actor1, const physx::PxTransform &frame1,
	physx::PxRigidActor *actor2, const physx::PxTransform &frame2);

} // namespace

} // namespace

#endif