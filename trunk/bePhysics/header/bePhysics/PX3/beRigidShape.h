/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_RIGIDSHAPE_PX
#define BE_PHYSICS_RIGIDSHAPE_PX

#include "bePhysics.h"
#include "../beRigidShape.h"
#include "beAssembledShape.h"
#include "beAPI.h"
#include <vector>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

namespace PX3
{

using beCore::PropertyDesc;

/// Mesh compound.
class RigidShape : public lean::nonassignable, 
	public beCore::ResourceToRefCounted< RigidShape, beCore::PropertyFeedbackProvider<bePhysics::RigidShape> >
{
public:
	/// Shape vector.
	typedef std::vector< physx::PxShape* > shape_vector;
	/// Subset vector.
	typedef std::vector< Subset > subset_vector;

private:
	lean::resource_ptr<const bePhysics::AssembledShape> m_pSource;

	shape_vector m_shapes;
	subset_vector m_subsets;

public:
	/// Empty compound constructor.
	BE_PHYSICS_PX_API RigidShape();
	/// Full compound constructor. Consecutive levels of detail have to reference consecutive subset ranges.
	BE_PHYSICS_PX_API RigidShape(physx::PxShape *const *shapeBegin, physx::PxShape *const *shapeEnd,
		Subset const *subsetBegin, Subset const *subsetEnd);
	/// Destructor.
	BE_PHYSICS_PX_API ~RigidShape();

	/// Gets all shapes.
	BE_PHYSICS_PX_API ShapeRange GetShapes() const LEAN_OVERRIDE { return beCore::MakeRangeN(ShapeHandle::Array(&m_shapes[0]), m_shapes.size()); }
	/// Gets all subsets.
	BE_PHYSICS_PX_API SubsetRange GetSubsets() const LEAN_OVERRIDE { return beCore::MakeRangeN(&m_subsets[0], m_subsets.size()); }

	/// Adds the given subset.
	BE_PHYSICS_PX_API uint4 AddSubset(const lean::utf8_ntri &name, bePhysics::Material *pMaterial = nullptr) LEAN_OVERRIDE;
	/// Sets the material for the given subset.
	BE_PHYSICS_PX_API void SetMaterial(uint4 subsetIdx, bePhysics::Material *pMaterial) LEAN_OVERRIDE;
	/// Removes the given subset.
	BE_PHYSICS_PX_API void RemoveSubset(uint4 subsetIdx) LEAN_OVERRIDE;

	/// Adds the given shape.
	BE_PHYSICS_PX_API uint4 AddShape(uint4 shapeIdx, bePhysics::ShapeHandle shape) LEAN_OVERRIDE;
	/// Removes the n-th shape.
	BE_PHYSICS_PX_API void RemoveShape(uint4 shapeIdx) LEAN_OVERRIDE;

	/// Sets the source shape.
	BE_PHYSICS_PX_API void SetSource(const bePhysics::AssembledShape *pSource) LEAN_OVERRIDE;
	/// Gets the source shape.
	BE_PHYSICS_PX_API const bePhysics::AssembledShape* GetSource() const LEAN_OVERRIDE { return m_pSource; }

	using bePhysics::RigidShape::GetComponentType;
	/// Gets the number of child components.
	BE_PHYSICS_PX_API uint4 GetComponentCount() const LEAN_OVERRIDE;
	/// Gets the name of the n-th child component.
	BE_PHYSICS_PX_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const LEAN_OVERRIDE;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_PHYSICS_PX_API lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> GetReflectedComponent(uint4 idx) const LEAN_OVERRIDE;

	/// Gets the type of the n-th child component.
	BE_PHYSICS_PX_API const beCore::ComponentType* GetComponentType(uint4 idx) const LEAN_OVERRIDE;
	/// Gets the n-th component.
	BE_PHYSICS_PX_API lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const LEAN_OVERRIDE;
	/// Returns true, if the n-th component can be replaced.
	BE_PHYSICS_PX_API bool IsComponentReplaceable(uint4 idx) const LEAN_OVERRIDE;
	/// Sets the n-th component.
	BE_PHYSICS_PX_API void SetComponent(uint4 idx, const lean::any &pComponent) LEAN_OVERRIDE;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }
};

template <> struct ToImplementationPX<bePhysics::RigidShape> { typedef RigidShape Type; };

/// Sets the given shape for the given actor.
BE_PHYSICS_PX_API void SetShape(physx::PxRigidActor &actor, const RigidShape &shape, const physx::PxMaterial *defaultMaterial);
/// Clones the shape of the given actor.
BE_PHYSICS_PX_API void CloneShape(physx::PxRigidActor &actor, const physx::PxRigidActor &shapeActor);

} // namespace

} // namespace

#endif