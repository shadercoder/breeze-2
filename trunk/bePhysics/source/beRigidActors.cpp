/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beShapes.h"
#include "bePhysics/PX/beRigidActors.h"
#include "bePhysics/PX/beDevice.h"
#include "bePhysics/PX/beMaterial.h"
#include "bePhysics/PX/beMath.h"
#include <PxExtensionsAPI.h>
#include <lean/io/raw_file.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace
{

// Adds all shapes in the given shape compound to the given actor.
void CreateShapesFromCompound(physx::PxRigidActor &actor, const ShapeCompoundPX &compound)
{
	const uint4 shapeCount = compound.GetShapeCount();

	for (uint4 i = 0; i < shapeCount; ++i)
		actor.createShape(
			*compound.GetGeometry()[i], 
			*compound.GetMaterials()[i], 
			compound.GetPoses()[i]);
}

} // namespace

// Creates a dynamic actor from the given shape.
lean::resource_ptr<RigidDynamic, true> CreateDynamicFromShape(Device &device, const ShapeCompound &shape, float mass)
{
	scoped_pxptr_t<physx::PxRigidDynamic>::t pRigid(
			ToImpl(device)->createRigidDynamic( physx::PxTransform::createIdentity() )
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("PxPhysics::createRigidDynamic()");

	CreateShapesFromCompound(*pRigid, ToImpl(shape));

	if (!physx::PxRigidBodyExt::setMassAndUpdateInertia(*pRigid, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");

	return lean::bind_resource<RigidDynamic>( new RigidDynamicPX(pRigid.detatch()) );
}

// Creates a dynamic box actor.
lean::resource_ptr<RigidDynamic, true> CreateDynamicBox(Device &device, const beMath::fvec3 &extents, Material &material, float mass,
	const beMath::fvec3 &pos, const beMath::fmat3 &orientation)
{
	scoped_pxptr_t<physx::PxRigidDynamic>::t pRigid(
			ToImpl(device)->createRigidDynamic( physx::PxTransform::createIdentity() )
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateDynamic()");

	if ( !pRigid->createShape( physx::PxBoxGeometry(extents[0], extents[1], extents[2]), *ToImpl(material), ToTransform(orientation, pos) ) )
		LEAN_THROW_ERROR_MSG("PxRigidDynamic::createShape()");

	if (!physx::PxRigidBodyExt::setMassAndUpdateInertia(*pRigid, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");

	return lean::bind_resource<RigidDynamic>( new RigidDynamicPX(pRigid.detatch()) );
}

// Creates a dynamic sphere actor.
lean::resource_ptr<RigidDynamic, true> CreateDynamicSphere(Device &device, float radius, Material &material, float mass,
	const beMath::fvec3 &center)
{
	scoped_pxptr_t<physx::PxRigidDynamic>::t pRigid(
			ToImpl(device)->createRigidDynamic( physx::PxTransform::createIdentity() )
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateDynamic()");

	if ( !pRigid->createShape( physx::PxSphereGeometry(radius), *ToImpl(material), ToTransform(center) ) )
		LEAN_THROW_ERROR_MSG("PxRigidDynamic::createShape()");

	if (!physx::PxRigidBodyExt::setMassAndUpdateInertia(*pRigid, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");

	return lean::bind_resource<RigidDynamic>( new RigidDynamicPX(pRigid.detatch()) );
}

// Creates a static actor from the given shape.
lean::resource_ptr<RigidStatic, true> CreateStaticFromShape(Device &device, const ShapeCompound &shape)
{
	scoped_pxptr_t<physx::PxRigidStatic>::t pRigid(
			ToImpl(device)->createRigidStatic( physx::PxTransform::createIdentity() )
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("PxPhysics::createRigidStatic()");

	CreateShapesFromCompound(*pRigid, ToImpl(shape));

	return lean::bind_resource<RigidStatic>( new RigidStaticPX(pRigid.detatch()) );
}

// Creates a static box actor.
lean::resource_ptr<RigidStatic, true> CreateStaticBox(Device &device, const beMath::fvec3 &extents, Material &material,
	const beMath::fvec3 &pos, const beMath::fmat3 &orientation)
{
	scoped_pxptr_t<physx::PxRigidStatic>::t pRigid(
			PxCreateStatic(
				*ToImpl(device),
				physx::PxTransform::createIdentity(),
				physx::PxBoxGeometry(extents[0], extents[1], extents[2]),
				*ToImpl(material),
				ToTransform(orientation, pos)
			)
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateStatic()");

	return lean::bind_resource<RigidStatic>( new RigidStaticPX(pRigid.detatch()) );
}

// Creates a static sphere actor.
lean::resource_ptr<RigidStatic, true> CreateStaticSphere(Device &device, float radius, Material &material,
	const beMath::fvec3 &center)
{
	scoped_pxptr_t<physx::PxRigidStatic>::t pRigid(
			PxCreateStatic(
				*ToImpl(device),
				physx::PxTransform::createIdentity(),
				physx::PxSphereGeometry(radius),
				*ToImpl(material),
				ToTransform(center)
			)
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateStatic()");

	return lean::bind_resource<RigidStatic>( new RigidStaticPX(pRigid.detatch()) );
}

// Sets filter data for the given actor.
void SetSimulationFilterData(RigidDynamic &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	SetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

// Sets filter data for the given actor.
void SetSimulationFilterData(RigidStatic &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	SetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

// Gets filter data from the given actor.
void GetSimulationFilterData(const RigidDynamic &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	GetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

// Gets filter data from the given actor.
void GetSimulationFilterData(const RigidStatic &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	GetSimulationFilterData( *ToImpl(actor), groupFlags, typeFlags, ID);
}

} // namespace