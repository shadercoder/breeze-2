/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_CHARACTERS_API_PX
#define BE_PHYSICS_CHARACTERS_API_PX

#include "bePhysics.h"
#include "beAPI.h"
#include <PxControllerManager.h>
#include <PxCapsuleController.h>

namespace bePhysics
{

namespace PX3
{

/// API types.
namespace API
{
	/// PhysX character scene.
	typedef physx::PxControllerManager CharacterScene;
	/// PhysX character controller.
	typedef physx::PxController Character;
	/// PhysX capsule character controller.
	typedef physx::PxCapsuleController CapsuleCharacter;

} // namespace

/// Converts the given vector to PhysX.
LEAN_INLINE physx::PxVec3 FromXAPI(const physx::PxExtendedVec3 &v)
{
	return physx::PxVec3(
		static_cast<physx::PxReal>(v.x),
		static_cast<physx::PxReal>(v.y),
		static_cast<physx::PxReal>(v.z) );
}

/// Converts the given vector to extended PhysX.
LEAN_INLINE physx::PxExtendedVec3 ToXAPI(const physx::PxVec3 &v)
{
	return physx::PxExtendedVec3( v.x, v.y, v.z );
}

} // namespace

} // namespace

#endif