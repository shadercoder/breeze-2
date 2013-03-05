/******************************************************/
/* breeze Engine Physics Module  (c) Tobias Zirr 2011 */
/******************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beAssembledShape.h"
#include "bePhysics/beRigidShape.h"

#include <beCore/beComponentTypes.h>

#include <algorithm>

namespace bePhysics
{

extern const beCore::ComponentType AssembledShapeType = { "AssembledShape" };

/// Gets the type.
const beCore::ComponentType* AssembledShape::GetComponentType()
{
	return &AssembledShapeType;
}

extern const beCore::ComponentType RigidShapeType = { "RigidShape" };

/// Gets the type.
const beCore::ComponentType* RigidShape::GetComponentType()
{
	return &RigidShapeType;
}
// Gets the type.
const beCore::ComponentType* RigidShape::GetType() const
{
	return &RigidShapeType;
}

/// Sorts the given shapes by name.
struct ShapeSorter
{
	bool operator ()(ShapeHandle left, ShapeHandle right) const
	{
		return GetName(left) < GetName(right);
	}
};

// Adds all shapes in the given assembled shape to the given rigid shape using the given material.
lean::resource_ptr<RigidShape, lean::critical_ref> ToRigidShape(const AssembledShape &src, Material *pMaterial, bool bStoreSource)
{
	lean::resource_ptr<RigidShape> rigidShape = CreateRigidShape();
	ToRigidShape(*rigidShape, src, pMaterial, bStoreSource);
	return rigidShape.transfer();
}

// Adds all shapes in the given assembled shape to the given rigid shape using the given material.
void ToRigidShape(RigidShape &dest, const AssembledShape &src, Material *pMaterial, bool bStoreSource)
{
	std::vector<ShapeHandle> shapes(src.GetShapeCount());
	uint4 shapeCount = src.GetShapes(&shapes[0]);
	std::sort(shapes.begin(), shapes.end(), ShapeSorter());

	uint4 subsetIdx;
	utf8_ntr lastSubsetName;
	
	for (uint4 i = 0; i < shapeCount; ++i)
	{
		ShapeHandle shape = shapes[i];
		utf8_ntr subsetName = GetName(shape);

		if (i == 0 || subsetName != lastSubsetName)
		{
			subsetIdx = dest.AddSubset(subsetName, pMaterial);
			lastSubsetName = subsetName;
		}

		dest.AddShape(subsetIdx, shape);
	}

	if (bStoreSource)
		dest.SetSource(&src);
}

// Fills in missing materials using the given material.
void FillRigidShape(RigidShape &shape, Material *material)
{
	RigidShape::SubsetRange subsets = shape.GetSubsets();

	for (uint4 i = 0, count = Size4(subsets); i < count; ++i)
		if (!subsets[i].Material)
			shape.SetMaterial(i, material);
}

// Transfers materials from one rigid shape to another, matching subset names.
bool TransferMaterials(const RigidShape &source, RigidShape &dest)
{
	bool bComplete = true;

	RigidShape::SubsetRange subsets = dest.GetSubsets();
	RigidShape::SubsetRange srcSubsets = source.GetSubsets();

	for (uint4 subsetIdx = 0, subsetCount = Size4(subsets); subsetIdx < subsetCount; ++subsetIdx)
	{
		const RigidShape::Subset &subset = subsets[subsetIdx];

		for (uint4 srcSubsetIdx = 0, srcSubsetCount = Size4(srcSubsets); srcSubsetIdx < srcSubsetCount; ++srcSubsetIdx)
			if (srcSubsets[srcSubsetIdx].Name == subset.Name)
				dest.SetMaterial(subsetIdx, srcSubsets[srcSubsetIdx].Material);

		if (!subset.Material)
			bComplete = false;
	}

	return bComplete;
}

} // namespace
