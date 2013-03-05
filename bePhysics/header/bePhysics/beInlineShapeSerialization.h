/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#pragma once
#ifndef BE_PHYSICS_INLINE_MESH_SERIALIZATION
#define BE_PHYSICS_INLINE_MESH_SERIALIZATION

#include "bePhysics.h"
#include <beCore/beParameterSet.h>
#include <beCore/beSerializationJobs.h>

#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

class Material;
class AssembledShape;
class RigidShape;
class ShapeCache;
class MaterialCache;

/// Schedules the given shape for inline serialization.
BE_PHYSICS_API void SaveShape(const AssembledShape *shape,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue);
/// Schedules the given shape for inline serialization.
BE_PHYSICS_API void SaveShape(const RigidShape *shape,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue);

/// Saves the given shape to the given XML node.
BE_PHYSICS_API void SaveShape(const RigidShape &mesh, rapidxml::xml_node<lean::utf8_t> &node);

/// Load the given shape from the given XML node.
BE_PHYSICS_API void LoadShape(RigidShape &shape, const rapidxml::xml_node<lean::utf8_t> &node,
							  ShapeCache &shapes, MaterialCache &materials, const Material *pDefaultMaterial);
/// Creates a shape from the given XML node.
BE_PHYSICS_API lean::resource_ptr<RigidShape, lean::critical_ref> LoadShape(const rapidxml::xml_node<lean::utf8_t> &node,
																			ShapeCache &shapes, MaterialCache &materials,
																			const Material *pDefaultMaterial);

} // namespace

#endif
