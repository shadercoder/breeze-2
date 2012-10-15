/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_MATH_PX
#define BE_PHYSICS_MATH_PX

#include "bePhysics.h"
#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>
#include <PxMath.h>
#include <PxVec2.h>
#include <PxVec3.h>
#include <PxVec4.h>
#include <PxMat33.h>
#include <PxMat44.h>

namespace bePhysics
{

namespace PX3
{

/// Converts the given vector to PhysX.
LEAN_INLINE physx::PxVec2 ToAPI(const beMath::fvec2 &v)
{
	return physx::PxVec2(v[0], v[1]);
}
/// Converts the given vector to PhysX.
LEAN_INLINE physx::PxVec3 ToAPI(const beMath::fvec3 &v)
{
	return physx::PxVec3(v[0], v[1], v[2]);
}
/// Converts the given vector to PhysX.
LEAN_INLINE physx::PxVec4 ToAPI(const beMath::fvec4 &v)
{
	return physx::PxVec4(v[0], v[1], v[2], v[3]);
}

/// Converts the given vector from PhysX.
LEAN_INLINE beMath::fvec2 FromAPI(const physx::PxVec2 &v)
{
	return beMath::vec(v.x, v.y);
}
/// Converts the given vector from PhysX.
LEAN_INLINE beMath::fvec3 FromAPI(const physx::PxVec3 &v)
{
	return beMath::vec(v.x, v.y, v.z);
}
/// Converts the given vector from PhysX.
LEAN_INLINE beMath::fvec4 FromAPI(const physx::PxVec4 &v)
{
	return beMath::vec(v.x, v.y, v.z, v.w);
}

/// Converts the given matrix to PhysX.
LEAN_INLINE physx::PxMat33 ToAPI(const beMath::fmat3 &m)
{
	return physx::PxMat33( const_cast<float*>(m.cdata()) );
}
/// Converts the given matrix to PhysX.
LEAN_INLINE physx::PxMat44 ToAPI(const beMath::fmat4 &m)
{
	return physx::PxMat44( const_cast<float*>(m.cdata()) );
}

/// Converts the given matrix from PhysX.
LEAN_INLINE beMath::fmat3 FromAPI(const physx::PxMat33 &m)
{
	return beMath::mat_row( FromAPI(m[0]), FromAPI(m[1]), FromAPI(m[2]) );
}
/// Converts the given matrix from PhysX.
LEAN_INLINE beMath::fmat4 FromAPI(const physx::PxMat44 &m)
{
	return beMath::mat_row( FromAPI(m[0]), FromAPI(m[1]), FromAPI(m[2]), FromAPI(m[3]) );
}

/// Converts the given orientation and position to PhysX.
LEAN_INLINE physx::PxTransform ToTransform(const beMath::fvec3 &p)
{
	return physx::PxTransform( ToAPI(p) );
}
/// Converts the given orientation and position to PhysX.
LEAN_INLINE physx::PxTransform ToTransform(const beMath::fmat3 &o)
{
	return physx::PxTransform( physx::PxQuat(ToAPI(o)) );
}
/// Converts the given orientation and position to PhysX.
LEAN_INLINE physx::PxTransform ToTransform(const beMath::fmat3 &o, const beMath::fvec3 &p)
{
	return physx::PxTransform( ToAPI(p), physx::PxQuat(ToAPI(o)) );
}

} // namespace

} // namespace

#endif