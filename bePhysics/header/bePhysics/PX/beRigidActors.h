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

/// Creates a dynamic rigid body.
BE_PHYSICS_PX_API physx::PxRigidDynamic* CreateRigidDynamic(physx::PxPhysics *pPhysics);
/// Creates a static rigid actor.
BE_PHYSICS_PX_API physx::PxRigidStatic* CreateRigidStatic(physx::PxPhysics *pPhysics);

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

// Prototypes
class Device;

/// Dynamic rigid body implementation.
class RigidDynamicPX : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxRigidDynamic, RigidDynamicPX> >, public RigidDynamic
{
private:
	scoped_pxptr_t<physx::PxRigidDynamic>::t m_pActor;

public:
	/// Constructor.
	BE_PHYSICS_PX_API RigidDynamicPX(Device &device);
	/// Constructor.
	BE_PHYSICS_PX_API RigidDynamicPX(physx::PxRigidDynamic *pActor);
	/// Destructor.
	BE_PHYSICS_PX_API ~RigidDynamicPX();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PXImplementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidDynamic*const& GetInterface() { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxRigidDynamic*const& GetInterface() const { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidDynamic*const& GetScene() { return m_pActor.get(); }
};

template <> struct ToImplementationPX<RigidDynamic> { typedef RigidDynamicPX Type; };

/// Static rigid actor implementation.
class RigidStaticPX : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxRigidStatic, RigidStaticPX> >, public RigidStatic
{
private:
	scoped_pxptr_t<physx::PxRigidStatic>::t m_pActor;

public:
	/// Constructor.
	BE_PHYSICS_PX_API RigidStaticPX(Device &device);
	/// Constructor.
	BE_PHYSICS_PX_API RigidStaticPX(physx::PxRigidStatic *pActor);
	/// Destructor.
	BE_PHYSICS_PX_API ~RigidStaticPX();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PXImplementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidStatic*const& GetInterface() { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxRigidStatic*const& GetInterface() const { return m_pActor.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidStatic*const& GetScene() { return m_pActor.get(); }
};

template <> struct ToImplementationPX<RigidStatic> { typedef RigidStaticPX Type; };

} // namespace

#endif