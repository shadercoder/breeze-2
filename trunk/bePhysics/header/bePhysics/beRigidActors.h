/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_RIGIDACTORS
#define BE_PHYSICS_RIGIDACTORS

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Serialization IDs
namespace RigidActorSerializationID
{
	/// Enum
	enum T
	{
		Actor = static_cast<uint4>(-2),				///< Dummy/Default Actor serialization ID.
		DefaultMaterial = static_cast<uint4>(-1),	///< Dummy/Default Material serialization ID.
		
		InternalBase = 0x80000000					///< Base for internal serialization IDs.
	};
}

/// Dynamic rigid body interface.
class RigidDynamic : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE RigidDynamic& operator =(const RigidDynamic&) { return *this; }

public:
	virtual ~RigidDynamic() throw() { }
};

/// Static rigid actor interface.
class RigidStatic : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE RigidStatic& operator =(const RigidStatic&) { return *this; }

public:
	virtual ~RigidStatic() throw() { }
};

// Prototypes
class Device;
class ShapeCompound;
class Material;

/// Creates a dynamic actor from the given shape.
BE_PHYSICS_API lean::resource_ptr<RigidDynamic, true> CreateDynamicFromShape(Device &device, const ShapeCompound &shape, float mass);
/// Creates a dynamic box actor.
BE_PHYSICS_API lean::resource_ptr<RigidDynamic, true> CreateDynamicBox(Device &device, const beMath::fvec3 &extents, Material &material, float mass,
	const beMath::fvec3 &pos = beMath::fvec3(0.0f), const beMath::fmat3 &orientation = beMath::fmat3::identity);
/// Creates a dynamic sphere actor.
BE_PHYSICS_API lean::resource_ptr<RigidDynamic, true> CreateDynamicSphere(Device &device, float radius, Material &material, float mass,
	const beMath::fvec3 &center = beMath::fvec3(0.0f));

/// Creates a static actor from the given shape.
BE_PHYSICS_API lean::resource_ptr<RigidStatic, true> CreateStaticFromShape(Device &device, const ShapeCompound &shape);
/// Creates a static box actor.
BE_PHYSICS_API lean::resource_ptr<RigidStatic, true> CreateStaticBox(Device &device, const beMath::fvec3 &extents, Material &material,
	const beMath::fvec3 &pos = beMath::fvec3(0.0f), const beMath::fmat3 &orientation = beMath::fmat3::identity);
/// Creates a static sphere actor.
BE_PHYSICS_API lean::resource_ptr<RigidStatic, true> CreateStaticSphere(Device &device, float radius, Material &material,
	const beMath::fvec3 &center = beMath::fvec3(0.0f));

/// Sets filter data for the given actor.
BE_PHYSICS_API void SetSimulationFilterData(RigidDynamic &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID);
/// Sets filter data for the given actor.
BE_PHYSICS_API void SetSimulationFilterData(RigidStatic &actor, uint4 groupFlags, uint4 typeFlags, uint4 ID);
/// Gets filter data from the given actor.
BE_PHYSICS_API void GetSimulationFilterData(const RigidDynamic &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID);
/// Gets filter data from the given actor.
BE_PHYSICS_API void GetSimulationFilterData(const RigidStatic &actor, int4 &groupFlags, uint4 &typeFlags, uint4 &ID);

} // namespace

#endif