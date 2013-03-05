/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_SHAPES_PX
#define BE_PHYSICS_SHAPES_PX

#include "bePhysics.h"
#include "../beShapes.h"
#include "beAPI.h"

namespace bePhysics
{

namespace PX3
{

/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxShape &shape, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxBoxGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxSphereGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxPlaneGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxCapsuleGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxConvexMeshGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxTriangleMeshGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);

} // namespace

} // namespace

#endif