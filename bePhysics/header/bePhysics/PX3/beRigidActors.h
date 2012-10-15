/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_RIGIDACTORS_PX
#define BE_PHYSICS_RIGIDACTORS_PX

#include "bePhysics.h"
#include "../beRigidActors.h"
#include <beCore/beWrapper.h>
#include <lean/tags/noncopyable.h>
#include "beAPI.h"

namespace bePhysics
{

// Prototypes
class Device;

namespace PX3
{

/// Creates a dynamic rigid body.
BE_PHYSICS_PX_API physx::PxRigidDynamic* CreateRigidDynamic(physx::PxPhysics &physics);
/// Creates a static rigid actor.
BE_PHYSICS_PX_API physx::PxRigidStatic* CreateRigidStatic(physx::PxPhysics &physics);

/// Adds the given shape.
BE_PHYSICS_PX_API void AddShape(physx::PxRigidActor &actor,
	const physx::PxGeometry &shape, const physx::PxMaterial *material, const physx::PxTransform &transform);

/// Sets the given mass.
BE_PHYSICS_PX_API void SetMass(physx::PxRigidBody &actor, float mass);
/// Sets the mass from the given density.
BE_PHYSICS_PX_API void SetDensity(physx::PxRigidBody &actor, float density);

/// Converts the given data into filter data.
LEAN_INLINE physx::PxFilterData ToFilterData(uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	return physx::PxFilterData(groupFlags, typeFlags, ID, 0);
}
/// Converts the given data into filter data.
LEAN_INLINE void ToFilterData(physx::PxFilterData &filterData, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	filterData.word0 = groupFlags;
	filterData.word1 = typeFlags;
	filterData.word2 = ID;
}
/// Gets the group flags from the given filter data.
LEAN_INLINE uint4 GetGroupFlags(const physx::PxFilterData &filterData) { return filterData.word0; }
/// Gets the group flags from the given filter data.
LEAN_INLINE uint4 GetTypeFlags(const physx::PxFilterData &filterData) { return filterData.word1; }
/// Gets the group flags from the given filter data.
LEAN_INLINE uint4 GetID(const physx::PxFilterData &filterData) { return filterData.word2; }

/// Sets filter data for the given actor.
BE_PHYSICS_PX_API void SetSimulationFilterData(physx::PxRigidActor &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID);
/// Gets filter data from the given actor.
BE_PHYSICS_PX_API void GetSimulationFilterData(const physx::PxRigidActor &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID);

/// Scales the given rigid actor as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxRigidActor &actor, const physx::PxVec3 &scaling);

/// Dynamic rigid body implementation.
class RigidDynamic : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxRigidDynamic, RigidDynamic> >, public bePhysics::RigidDynamic
{
private:
	scoped_pxptr_t<physx::PxRigidDynamic>::t m_pActor;

public:
	/// Constructor.
	BE_PHYSICS_PX_API RigidDynamic(bePhysics::Device &device);
	/// Constructor.
	BE_PHYSICS_PX_API RigidDynamic(physx::PxRigidDynamic *actor);
	/// Destructor.
	BE_PHYSICS_PX_API ~RigidDynamic();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidDynamic*const& GetInterface() { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxRigidDynamic*const& GetInterface() const { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidDynamic*const& GetScene() { return m_pActor.get(); }
};

template <> struct ToImplementationPX<bePhysics::RigidDynamic> { typedef RigidDynamic Type; };

/// Static rigid actor implementation.
class RigidStatic : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxRigidStatic, RigidStatic> >, public bePhysics::RigidStatic
{
private:
	scoped_pxptr_t<physx::PxRigidStatic>::t m_pActor;

public:
	/// Constructor.
	BE_PHYSICS_PX_API RigidStatic(bePhysics::Device &device);
	/// Constructor.
	BE_PHYSICS_PX_API RigidStatic(physx::PxRigidStatic *actor);
	/// Destructor.
	BE_PHYSICS_PX_API ~RigidStatic();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidStatic*const& GetInterface() { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxRigidStatic*const& GetInterface() const { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidStatic*const& GetScene() { return m_pActor.get(); }
};

template <> struct ToImplementationPX<bePhysics::RigidStatic> { typedef RigidStatic Type; };

class ShapeCompound;

/// Adds all shapes in the given shape compound to the given actor.
BE_PHYSICS_PX_API void CreateShapesFromCompound(physx::PxRigidActor &actor, const ShapeCompound &compound);

} // namespace

} // namespace

#endif