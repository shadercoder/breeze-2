/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_RIGIDSHAPE
#define BE_PHYSICS_RIGIDSHAPE

#include "bePhysics.h"
#include "beAssembledShape.h"
#include "beMaterial.h"
#include <beCore/beShared.h>
#include <beCore/beManagedResource.h>
#include <beCore/beComponent.h>
#include <beCore/beReflectedComponent.h>
#include <beCore/beMany.h>

namespace bePhysics
{

class Material;
class AssembledShape;
class RigidShapeCache;

using beCore::PropertyDesc;

/// Mesh compound.
class RigidShape : public beCore::Resource, public beCore::OptionalPropertyProvider<beCore::ReflectedComponent>,
	public beCore::ManagedResource<RigidShapeCache>, public beCore::HotResource<RigidShape>, public Implementation
{
	LEAN_SHARED_INTERFACE_BEHAVIOR(RigidShape)

public:
	/// Subset.
	struct Subset
	{
		utf8_string Name;						///< Subset name.
		beCore::Range<uint4> Shapes;			///< Subset shape range.
		lean::resource_ptr<Material> Material;	///< Subset material.
	};

	typedef beCore::Range<ShapeHandle const*> ShapeRange;
	typedef beCore::Range<Subset const*> SubsetRange;

	/// Gets all shapes.
	virtual ShapeRange GetShapes() const = 0;
	/// Gets all subsets.
	virtual SubsetRange GetSubsets() const = 0;

	/// Adds the given subset.
	virtual uint4 AddSubset(const lean::utf8_ntri &name, Material *pMaterial = nullptr) = 0;
	/// Sets the material for the given subset.
	virtual void SetMaterial(uint4 subsetIdx, Material *pMaterial) = 0;
	/// Removes the given subset.
	virtual void RemoveSubset(uint4 subsetIdx) = 0;

	/// Adds the given shape.
	virtual uint4 AddShape(uint4 subsetIdx, ShapeHandle shape) = 0;
	/// Removes the n-th shape.
	virtual void RemoveShape(uint4 shapeIdx) = 0;

	/// Sets the source shape.
	virtual void SetSource(const AssembledShape *pSource) = 0;
	/// Gets the source shape.
	virtual const AssembledShape* GetSource() const = 0;

	/// Gets the type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const;
};

/// Creates a rigid shape.
BE_PHYSICS_API lean::resource_ptr<RigidShape, lean::critical_ref> CreateRigidShape();
/// Adds all shapes in the given assembled shape to the given rigid shape using the given material.
BE_PHYSICS_API lean::resource_ptr<RigidShape, lean::critical_ref> ToRigidShape(
	const AssembledShape &src, Material *pMaterial, bool bStoreSource);
/// Adds all shapes in the given assembled shape to the given rigid shape using the given material.
BE_PHYSICS_API void ToRigidShape(RigidShape &dest, const AssembledShape &src, Material *pMaterial, bool bStoreSource);
/// Fills in missing materials using the given material.
BE_PHYSICS_API void FillRigidShape(RigidShape &shape, Material *material);
/// Transfers materials from one rigid shape to another, matching subset names.
BE_PHYSICS_API bool TransferMaterials(const RigidShape &source, RigidShape &dest);

} // namespace

#endif