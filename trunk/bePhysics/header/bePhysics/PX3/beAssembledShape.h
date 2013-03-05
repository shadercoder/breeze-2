/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_ASSEMBLEDSHAPE_PX
#define BE_PHYSICS_ASSEMBLEDSHAPE_PX

#include "bePhysics.h"
#include "../beAssembledShape.h"
#include <beCore/beWrapper.h>
#include <lean/tags/noncopyable.h>
#include "beAPI.h"

BE_CORE_DEFINE_QUALIFIED_HANDLE(bePhysics::ShapeTag, physx::PxShape, beCore::IntransitiveWrapper);

namespace bePhysics
{

namespace PX3
{

class Device;

/// Shape handle.
typedef beCore::QualifiedHandle<ShapeTag> ShapeHandle;

/// Dynamic rigid body implementation.
class AssembledShape : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxRigidDynamic, AssembledShape> >, public bePhysics::AssembledShape
{
private:
	void *m_pSerialDataToBeFreed;
	scoped_pxptr_t<physx::PxRigidDynamic>::t m_actorPrototype;

public:
	/// Constructor.
	BE_PHYSICS_PX_API AssembledShape(physx::PxPhysics &device);
	/// Constructor.
	BE_PHYSICS_PX_API AssembledShape(physx::PxRigidDynamic *actor, void *pSerialDataToBeFreed = nullptr);
	/// Destructor.
	BE_PHYSICS_PX_API ~AssembledShape();

	/// Gets the number of shapes.
	BE_PHYSICS_PX_API uint4 GetShapeCount() const;
	/// Gets the name of the n-th shape.
	BE_PHYSICS_PX_API utf8_ntr GetShapeName(uint4 idx) const;
	/// Gets the n-th shape.
	BE_PHYSICS_PX_API bePhysics::ShapeHandle GetShape(uint4 idx) const;
	/// Gets the at max given number of shapes.
	BE_PHYSICS_PX_API uint4 GetShapes(bePhysics::ShapeHandle *shapes, uint4 count = -1, uint4 offset = 0) const;

	/// Sets the serialization data to be freed.
	LEAN_INLINE void SetSerializationDataToBeFreed(void *pData) { m_pSerialDataToBeFreed = pData; }

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxRigidDynamic*const& GetInterface() { return m_actorPrototype.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxRigidDynamic*const& GetInterface() const { return m_actorPrototype.get(); }
};

template <> struct ToImplementationPX<bePhysics::AssembledShape> { typedef AssembledShape Type; };

} // namespace

} // namespace

#endif