/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beRigidShape.h"
#include "bePhysics/PX3/beAssembledShape.h"

#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beMaterial.h"

#include <lean/functional/algorithm.h>
#include <lean/io/numeric.h>

namespace bePhysics
{

namespace PX3
{

// Constructor,
RigidShape::RigidShape()
{
}

// Constructor,
RigidShape::RigidShape(physx::PxShape *const *shapeBegin, physx::PxShape *const *shapeEnd,
		Subset const *subsetBegin, Subset const *subsetEnd)
	: m_shapes(shapeBegin, shapeEnd),
	m_subsets(subsetBegin, subsetEnd)
{
	uint4 nextSubsetOffset = 0;

	// Verify subset integrity
	for (subset_vector::iterator it = m_subsets.begin(), itEnd = m_subsets.end(); it != itEnd; ++it)
	{
		// Consecutive ranges of subsets required
		LEAN_ASSERT(it->Shapes.Begin == nextSubsetOffset);
		LEAN_ASSERT(it->Shapes.Begin <= it->Shapes.End);
		// MONITOR: Throw instead?!

		nextSubsetOffset = it->Shapes.End;
	}

	// Make sure all meshes are contained
	LEAN_ASSERT(nextSubsetOffset == m_shapes.size());
}

// Destructor.
RigidShape::~RigidShape()
{
}

// Adds the given subset.
uint4 RigidShape::AddSubset(const lean::utf8_ntri &name, bePhysics::Material *pMaterial)
{
	uint4 subsetIdx = (uint4) m_subsets.size();
	{
		uint4 nextShapeIdx = (uint4) m_shapes.size();
		Subset subset = { name.to<utf8_string>(), bec::MakeRange(nextShapeIdx, nextShapeIdx), pMaterial };
		m_subsets.push_back( LEAN_MOVE(subset) );
	}
	return subsetIdx;
}

// Sets the material for the given subset.
void RigidShape::SetMaterial(uint4 subsetIdx, bePhysics::Material *pMaterial)
{
	LEAN_ASSERT(subsetIdx < m_subsets.size());
	m_subsets[subsetIdx].Material = pMaterial;
}

// Removes the given subset.
void RigidShape::RemoveSubset(uint4 subsetIdx)
{
	LEAN_ASSERT(subsetIdx < m_subsets.size());
	
	// Remove subset shapes
	{
		Subset &subset = m_subsets[subsetIdx];
		uint4 removedCount = Size4(subset.Shapes);
		m_shapes.erase(m_shapes.begin() + subset.Shapes.Begin, m_shapes.begin() + subset.Shapes.End);
		subset.Shapes.End = subset.Shapes.Begin;

		// Update subsequent subsets
		for (subset_vector::iterator it = m_subsets.begin() + subsetIdx, itEnd = m_subsets.end(); ++it != itEnd; )
		{
			it->Shapes.Begin -= removedCount;
			it->Shapes.End -= removedCount;
		}
	}

	// Erase subset
	m_subsets.erase(m_subsets.begin() + subsetIdx);
}

// Adds the given shape.
uint4 RigidShape::AddShape(uint4 subsetIdx, bePhysics::ShapeHandle shapeHandle)
{
	physx::PxShape *shape = ToImpl(shapeHandle);
	LEAN_ASSERT(shape);
	LEAN_ASSERT(subsetIdx < m_subsets.size());

	// Insert shape at the end of the given subset
	Subset &subset = m_subsets[subsetIdx];
	uint4 shapeIdx = subset.Shapes.End;
	m_shapes.insert(m_shapes.begin() + shapeIdx, shape);
	++subset.Shapes.End;

	// Update subsequent subsets
	for (subset_vector::iterator it = m_subsets.begin() + subsetIdx, itEnd = m_subsets.end(); ++it != itEnd; )
	{
		++it->Shapes.Begin;
		++it->Shapes.End;
	}

	return shapeIdx;
}

// Removes the n-th shape.
void RigidShape::RemoveShape(uint4 shapeIdx)
{
	LEAN_ASSERT(shapeIdx < m_shapes.size());
	m_shapes.erase(m_shapes.begin() + shapeIdx);

	// Update subsequent subsets
	for (subset_vector::iterator it = m_subsets.begin(), itEnd = m_subsets.end(); it != itEnd; ++it)
	{
		it->Shapes.Begin -= (it->Shapes.Begin > shapeIdx);
		it->Shapes.End -= (it->Shapes.End > shapeIdx);
	}
}

// Sets the source shape.
void RigidShape::SetSource(const bePhysics::AssembledShape *pSource)
{
	m_pSource = pSource;
}

// Gets the number of child components.
uint4 RigidShape::GetComponentCount() const
{
	return static_cast<uint4>( m_subsets.size() );
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string RigidShape::GetComponentName(uint4 idx) const
{
	beCore::Exchange::utf8_string name;

	LEAN_ASSERT(idx < m_subsets.size());
	const utf8_string &subsetName = m_subsets[idx].Name;
	utf8_string subsetNum = lean::int_to_string(idx);
	
	bool bNamed = !subsetName.empty();
	name.reserve(
			lean::ntarraylen("Subset ") + subsetNum.size()
			 + subsetName.size() + lean::ntarraylen(" ()") * bNamed
		);

	name.append(subsetName.begin(), subsetName.end());
	if (bNamed)
		name.append(" (");
	name.append("Subset ");
	name.append(subsetNum.begin(), subsetNum.end());
	if (bNamed)
		name.append(")");

	return name;
}

// Gets the n-th reflected child component, nullptr if not reflected.
lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> RigidShape::GetReflectedComponent(uint4 idx) const
{
	LEAN_ASSERT(idx < m_subsets.size());
	return Reflect(m_subsets[idx].Material);
}

// Gets the type of the n-th child component.
const beCore::ComponentType* RigidShape::GetComponentType(uint4 idx) const
{
	return Material::GetComponentType();
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> RigidShape::GetComponent(uint4 idx) const
{
	return bec::any_resource_t<bePhysics::Material>::t( m_subsets[idx].Material );
}

// Returns true, if the n-th component can be replaced.
bool RigidShape::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void RigidShape::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetMaterial( idx, any_cast<bePhysics::Material*>(pComponent) );
}

// Sets the given shape for the given actor.
void SetShape(physx::PxRigidActor &actor, const RigidShape &shapeCompound, const physx::PxMaterial *defaultMaterial)
{
	physx::PxShape *pRemainingShape = ClearShapes(actor);

	RigidShape::ShapeRange shapes = shapeCompound.GetShapes();
	
	// Add shapes grouped by subset
	for (RigidShape::SubsetRange subset = shapeCompound.GetSubsets(); subset; ++subset)
	{
		// Override placeholder materials with subset material
		const physx::PxMaterial *pMaterial = defaultMaterial;
		if (subset->Material)
			pMaterial = ToImpl(*subset->Material);
		
		// Add subset shapes
		for (uint4 shapeIdx = subset->Shapes.Begin, shapeEndIdx = subset->Shapes.End; shapeIdx < shapeEndIdx; ++shapeIdx)
		{
			const physx::PxShape *shape = ToImpl(shapes[shapeIdx]);

			const physx::PxMaterial *shapeMaterial = pMaterial;
			if (!shapeMaterial)
				shape->getMaterials(const_cast<physx::PxMaterial**>(&shapeMaterial), 1);

			actor.createShape(shape->getGeometry().any(), *shapeMaterial, shape->getLocalPose());
		}
	}

	// NOTE: PhysX not always able to remove all shapes
	if (pRemainingShape)
		pRemainingShape->release();
}

// Clones the shape of the given actor.
void CloneShape(physx::PxRigidActor &actor, const physx::PxRigidActor &shapeActor)
{
	physx::PxShape *pRemainingShape = ClearShapes(actor);

	uint4 shapeCount = shapeActor.getNbShapes();

	for (uint4 i = 0; i < shapeCount; ++i)
	{
		physx::PxShape *shape;
		shapeActor.getShapes(&shape, 1, i);
		LEAN_ASSERT(shape);
		physx::PxMaterial *material;
		shape->getMaterials(&material, 1);
		LEAN_ASSERT(material);
		actor.createShape(shape->getGeometry().any(), *material, shape->getLocalPose());
	}

	// NOTE: PhysX not always able to remove all shapes
	if (pRemainingShape)
		pRemainingShape->release();
}

} // namespace

// Creates a rigid shape.
lean::resource_ptr<RigidShape, lean::critical_ref> CreateRigidShape()
{
	return new_resource PX3::RigidShape();
}

} // namespace