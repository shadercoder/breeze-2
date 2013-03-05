/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beRigidShape.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beMath.h"
#include <PxExtensionsAPI.h>
#include <lean/io/raw_file.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

// Creates a dynamic actor from the given shape.
lean::resource_ptr<RigidDynamic, true> CreateDynamicFromShape(Device &device, const RigidShape &shape, float mass)
{
	PX3::scoped_pxptr_t<physx::PxRigidDynamic>::t pRigid(
			ToImpl(device)->createRigidDynamic( physx::PxTransform::createIdentity() )
		);
	if (!pRigid)
		LEAN_THROW_ERROR_MSG("PxPhysics::createRigidDynamic()");

	SetShape(*pRigid, ToImpl(shape), nullptr);

	if (mass > 0.0f && !physx::PxRigidBodyExt::setMassAndUpdateInertia(*pRigid, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");
	else if (!physx::PxRigidBodyExt::updateMassAndInertia(*pRigid, -mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::updateMassAndInertia()");

	return lean::bind_resource<RigidDynamic>( new PX3::RigidDynamic(pRigid.detach()) );
}

// Creates a dynamic box actor.
lean::resource_ptr<RigidDynamic, true> CreateDynamicBox(Device &device, const beMath::fvec3 &extents, Material *material, float mass,
	const beMath::fvec3 &pos, const beMath::fmat3 &orientation)
{
	LEAN_ASSERT_NOT_NULL(material);

	PX3::scoped_pxptr_t<physx::PxRigidDynamic>::t pRigid(
			ToImpl(device)->createRigidDynamic( physx::PxTransform::createIdentity() )
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateDynamic()");

	if ( !pRigid->createShape( physx::PxBoxGeometry(extents[0], extents[1], extents[2]), **ToImpl(material), PX3::ToTransform(orientation, pos) ) )
		LEAN_THROW_ERROR_MSG("PxRigidDynamic::createShape()");

	if (mass > 0.0f && !physx::PxRigidBodyExt::setMassAndUpdateInertia(*pRigid, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");
	else if (!physx::PxRigidBodyExt::updateMassAndInertia(*pRigid, -mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::updateMassAndInertia()");

	return lean::bind_resource<RigidDynamic>( new PX3::RigidDynamic(pRigid.detach()) );
}

// Creates a dynamic sphere actor.
lean::resource_ptr<RigidDynamic, true> CreateDynamicSphere(Device &device, float radius, Material *material, float mass,
	const beMath::fvec3 &center)
{
	LEAN_ASSERT_NOT_NULL(material);

	PX3::scoped_pxptr_t<physx::PxRigidDynamic>::t pRigid(
			ToImpl(device)->createRigidDynamic( physx::PxTransform::createIdentity() )
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateDynamic()");

	if ( !pRigid->createShape( physx::PxSphereGeometry(radius), **ToImpl(material), PX3::ToTransform(center) ) )
		LEAN_THROW_ERROR_MSG("PxRigidDynamic::createShape()");

	if (mass > 0.0f && !physx::PxRigidBodyExt::setMassAndUpdateInertia(*pRigid, mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::setMassAndUpdateInertia()");
	else if (!physx::PxRigidBodyExt::updateMassAndInertia(*pRigid, -mass))
		LEAN_THROW_ERROR_MSG("PxRigidBodyExt::updateMassAndInertia()");

	return lean::bind_resource<RigidDynamic>( new PX3::RigidDynamic(pRigid.detach()) );
}

// Creates a static actor from the given shape.
lean::resource_ptr<RigidStatic, true> CreateStaticFromShape(Device &device, const RigidShape &shape)
{
	PX3::scoped_pxptr_t<physx::PxRigidStatic>::t pRigid(
			ToImpl(device)->createRigidStatic( physx::PxTransform::createIdentity() )
		);
	if (!pRigid)
		LEAN_THROW_ERROR_MSG("PxPhysics::createRigidStatic()");

	SetShape(*pRigid, ToImpl(shape), nullptr);

	return lean::bind_resource<RigidStatic>( new PX3::RigidStatic(pRigid.detach()) );
}

// Creates a static box actor.
lean::resource_ptr<RigidStatic, true> CreateStaticBox(Device &device, const beMath::fvec3 &extents, Material *material,
	const beMath::fvec3 &pos, const beMath::fmat3 &orientation)
{
	LEAN_ASSERT_NOT_NULL(material);

	PX3::scoped_pxptr_t<physx::PxRigidStatic>::t pRigid(
			PxCreateStatic(
				*ToImpl(device),
				physx::PxTransform::createIdentity(),
				physx::PxBoxGeometry(extents[0], extents[1], extents[2]),
				**ToImpl(material),
				PX3::ToTransform(orientation, pos)
			)
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateStatic()");

	return lean::bind_resource<RigidStatic>( new PX3::RigidStatic(pRigid.detach()) );
}

// Creates a static sphere actor.
lean::resource_ptr<RigidStatic, true> CreateStaticSphere(Device &device, float radius, Material *material,
	const beMath::fvec3 &center)
{
	LEAN_ASSERT_NOT_NULL(material);

	PX3::scoped_pxptr_t<physx::PxRigidStatic>::t pRigid(
			PxCreateStatic(
				*ToImpl(device),
				physx::PxTransform::createIdentity(),
				physx::PxSphereGeometry(radius),
				**ToImpl(material),
				PX3::ToTransform(center)
			)
		);

	if (!pRigid)
		LEAN_THROW_ERROR_MSG("physx::PxCreateStatic()");

	return lean::bind_resource<RigidStatic>( new PX3::RigidStatic(pRigid.detach()) );
}

} // namespace