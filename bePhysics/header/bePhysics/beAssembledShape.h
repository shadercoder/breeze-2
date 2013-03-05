/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_ASSEMBLEDSHAPE
#define BE_PHYSICS_ASSEMBLEDSHAPE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beCore/beManagedResource.h>
#include <beCore/beComponent.h>
#include <beCore/beOpaqueHandle.h>

namespace bePhysics
{

class ShapeCache;

struct ShapeTag;
/// Shape handle.
typedef beCore::OpaqueHandle<ShapeTag> ShapeHandle;
using beCore::ToImpl;

/// Gets the name of the given shape.
BE_PHYSICS_API utf8_ntr GetName(ShapeHandle shape);

/// Dynamic rigid body interface.
class AssembledShape : public beCore::Resource,
	public beCore::ManagedResource<ShapeCache>, public beCore::HotResource<AssembledShape>, public Implementation
{
	LEAN_SHARED_INTERFACE_BEHAVIOR(AssembledShape)

public:
	/// Gets the number of shapes.
	virtual uint4 GetShapeCount() const = 0;
	/// Gets the name of the n-th shape.
	virtual utf8_ntr GetShapeName(uint4 idx) const = 0;
	/// Gets the n-th shape.
	virtual ShapeHandle GetShape(uint4 idx) const = 0;
	/// Gets the at max given number of shapes.
	virtual uint4 GetShapes(ShapeHandle *shapes, uint4 count = -1, uint4 offset = 0) const = 0;

	/// Gets the component type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
};

// Prototypes
class Device;
class Material;

/// Creates a shape from the given shape data block.
BE_PHYSICS_API lean::resource_ptr<AssembledShape, lean::critical_ref> LoadShape(const void *data, uint8 dataLength, const Material *pMaterial, Device &device);
/// Creates a shape from the given shape file.
BE_PHYSICS_API lean::resource_ptr<AssembledShape, lean::critical_ref> LoadShape(const utf8_ntri &file, const Material *pMaterial, Device &device);

} // namespace

#endif