/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beInlineShapeSerialization.h"
#include "bePhysics/beInlineMaterialSerialization.h"

#include <beEntitySystem/beSerializationParameters.h>
#include <beEntitySystem/beSerializationTasks.h>

#include "bePhysics/beSerializationParameters.h"
#include "bePhysics/beResourceManager.h"

#include "bePhysics/beAssembledShape.h"
#include "bePhysics/beRigidShape.h"
#include "bePhysics/beShapeCache.h"
#include "bePhysics/beRigidShapeCache.h"
#include "bePhysics/beMaterialCache.h"

#include "bePhysics/beInlineMaterialSerialization.h"

#include <lean/xml/utility.h>
#include <lean/xml/numeric.h>

#include <set>

#include <lean/logging/errors.h>

namespace bePhysics
{

// Saves the given shape to the given XML shape.
void SaveShape(const RigidShape &shape, rapidxml::xml_node<lean::utf8_t> &node)
{
	rapidxml::xml_document<utf8_t> &document = *node.document();

	const AssembledShape *pSourceShape = shape.GetSource();
	utf8_ntr sourceName = bec::GetCachedName<utf8_ntr>(pSourceShape);

	// Tied to one source shape?
	if (!sourceName.empty())
		lean::append_attribute(document, node, "source", sourceName);
	else
	{
		LEAN_LOG_ERROR_MSG("Could not identify source shape, will be lost");
		return;
	}

	// Subsets
	for (RigidShape::SubsetRange subsets = shape.GetSubsets(); subsets; ++subsets)
	{
		rapidxml::xml_node<utf8_t> &subsetNode = *lean::allocate_node<utf8_t>(document, "s");
		lean::append_attribute(document, subsetNode, "n", subsets->Name);
		node.append_node(&subsetNode);

		utf8_ntr materialName = bec::GetCachedName<utf8_ntr>(subsets->Material.get());
		if (!materialName.empty())
			lean::append_attribute(document, subsetNode, "material", materialName);
		else
			LEAN_LOG_ERROR_CTX("Could not identify material, will be lost", subsets->Name);
	}
}

// Load the given shape from the given XML node.
void LoadShape(RigidShape &shape, const rapidxml::xml_node<lean::utf8_t> &node,
			   ShapeCache &shapes, MaterialCache &materials)
{
	// Shape (single or distributed)
	utf8_ntr sourceName = lean::get_attribute(node, "source");
	const AssembledShape *sourceShape = shapes.GetByName(sourceName, true);

	ToRigidShape(shape, *sourceShape, nullptr, true);
	RigidShape::SubsetRange subsets = shape.GetSubsets();
	
	// Load subsets
	for (const rapidxml::xml_node<utf8_t> *subsetNode = node.first_node("s");
		subsetNode; subsetNode = subsetNode->next_sibling("s"))
	{
		utf8_ntr subsetName = lean::get_attribute(*subsetNode, "n"); 
		bool bSubsetIdentified = false;

		for (uint4 subsetIdx = 0, subsetCount = Size4(subsets); subsetIdx < subsetCount; ++subsetIdx)
		{
			const RigidShape::Subset &subset = subsets[subsetIdx];

			// Find first matching subset with unset material
			if (subsets[subsetIdx].Name == subsetName && !shape.GetSubsets()[subsetIdx].Material)
			{
				shape.SetMaterial(
						subsetIdx,
						materials.GetByName( lean::get_attribute(*subsetNode, "material"), true )
					);
				bSubsetIdentified = true;
				break;
			}
		}
		
		if (!bSubsetIdentified)
			LEAN_LOG_ERROR_CTX("Subset could not be matched, information will be lost", subsetName);
	}
}

// Creates a shape from the given XML node.
lean::resource_ptr<RigidShape, lean::critical_ref> LoadShape(const rapidxml::xml_node<lean::utf8_t> &node,
															 ShapeCache &shapes, MaterialCache &materials)
{
	lean::resource_ptr<RigidShape> shape = CreateRigidShape();
	LoadShape(*shape, node, shapes, materials);
	return shape.transfer();
}

namespace
{


/// Serializes a list of materials.
class ShapeImportSerializer : public beCore::SaveJob
{
private:
	typedef std::set<const AssembledShape*> shape_set;
	shape_set m_shapes;

public:
	/// Adds the given material for serialization.
	void Add(const AssembledShape *shape)
	{
		LEAN_ASSERT_NOT_NULL( shape );
		LEAN_ASSERT_NOT_NULL( shape->GetCache() ); // TODO: Exceptions + check source & source cache
		m_shapes.insert(shape);
	}

	/// Saves anything, e.g. to the given XML root node.
	void Save(rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const
	{
		rapidxml::xml_document<utf8_t> &document = *root.document();

		rapidxml::xml_node<utf8_t> &shapesNode = *lean::allocate_node<utf8_t>(document, "shapes");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		root.append_node(&shapesNode);

		for (shape_set::const_iterator itShape = m_shapes.begin(); itShape != m_shapes.end(); itShape++)
		{
			const AssembledShape *shape = *itShape;
			const ShapeCache *cache = shape->GetCache();
			utf8_ntr name = cache->GetName(shape);
			utf8_ntr file = cache->GetFile(shape);
			
			if (name.empty())
				LEAN_LOG_ERROR_CTX("Imported shape missing name, will be lost", file);
			else if (file.empty())
				LEAN_LOG_ERROR_CTX("Imported shape missing file, will be lost", name);
			else
			{
				rapidxml::xml_node<utf8_t> &shapeNode = *lean::allocate_node<utf8_t>(document, "s");
				lean::append_attribute( document, shapeNode, "name", name );
				lean::append_attribute( document, shapeNode, "file", cache->GetPathResolver().Shorten(file) );
				shapesNode.append_node(&shapeNode);
			}
		}
	}
};

/// Serializes a list of materials.
class ShapeSerializer : public beCore::SaveJob
{
private:
	typedef std::set<const RigidShape*> shape_set;
	shape_set m_shapes;

public:
	/// Adds the given material for serialization.
	void Add(const RigidShape *shape)
	{
		LEAN_ASSERT_NOT_NULL( shape );
		LEAN_ASSERT_NOT_NULL( shape->GetCache() ); // TODO: Exceptions + check source & source cache
		m_shapes.insert(shape);
	}

	/// Saves anything, e.g. to the given XML root node.
	void Save(rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const
	{
		rapidxml::xml_document<utf8_t> &document = *root.document();

		rapidxml::xml_node<utf8_t> &shapesNode = *lean::allocate_node<utf8_t>(document, "rigidshapes");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		root.append_node(&shapesNode);

		for (shape_set::const_iterator itShape = m_shapes.begin(); itShape != m_shapes.end(); itShape++)
		{
			const RigidShape *shape = *itShape;
			const RigidShapeCache *cache = shape->GetCache();
			utf8_ntr name = cache->GetName(shape);

			if (name.empty())
				LEAN_LOG_ERROR_MSG("Rigid shape missing name, will be lost");
			else
			{
				rapidxml::xml_node<utf8_t> &shapeNode = *lean::allocate_node<utf8_t>(document, "s");
				lean::append_attribute( document, shapeNode, "name", name );
				// ORDER: Append FIRST, otherwise parent document == nullptr
				shapesNode.append_node(&shapeNode);

				SaveShape(*shape, shapeNode);
			}
		}
	}
};

const uint4 ShapeImportSerializerID = beEntitySystem::GetSerializationParameters().Add("bePhysics.ShapeImportSerializer");
const uint4 ShapeSerializerID = beEntitySystem::GetSerializationParameters().Add("bePhysics.ShapeSerializer");

} // namespace

// Schedules the given shape for inline serialization.
void SaveShape(const AssembledShape *shape, beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue)
{
	ShapeImportSerializer *pSerializer = parameters.GetValueDefault< ShapeImportSerializer* >(beEntitySystem::GetSerializationParameters(), ShapeImportSerializerID);

	// Create serializer on first call
	if (!pSerializer)
	{
		// NOTE: Serialization queue takes ownership
		pSerializer = new ShapeImportSerializer();
		queue.AddSerializationJob(pSerializer);
		parameters.SetValue< ShapeImportSerializer* >(beEntitySystem::GetSerializationParameters(), ShapeImportSerializerID, pSerializer);
	}

	// Schedule material for serialization
	pSerializer->Add(shape);
}

// Schedules the given material for inline serialization.
void SaveShape(const RigidShape *shape, beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue)
{
	ShapeSerializer *pSerializer = parameters.GetValueDefault< ShapeSerializer* >(beEntitySystem::GetSerializationParameters(), ShapeSerializerID);

	// Create serializer on first call
	if (!pSerializer)
	{
		// NOTE: Serialization queue takes ownership
		pSerializer = new ShapeSerializer();
		queue.AddSerializationJob(pSerializer);
		parameters.SetValue< ShapeSerializer* >(beEntitySystem::GetSerializationParameters(), ShapeSerializerID, pSerializer);
	}

	// Schedule material for serialization
	pSerializer->Add(shape);

	// Schedule shapes for serialization
	if (const AssembledShape *sourceShape = shape->GetSource())
		SaveShape(sourceShape, parameters, queue);

	// Schedule materials for serialization
	for (RigidShape::SubsetRange subsets = shape->GetSubsets(); subsets; ++subsets)
		if (subsets->Material)
			SaveMaterial(subsets->Material, parameters, queue);
}

namespace
{

/// Imports a list of shapes.
class ShapeImportLoader : public beCore::LoadJob
{
public:
	/// Loads anything, e.g. to the given XML root node.
	void Load(const rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
		ShapeCache &shapeCache = *LEAN_ASSERT_NOT_NULL(physicsParameters.ResourceManager)->ShapeCache();
		
		bool bNoOverwrite = beEntitySystem::GetNoOverwriteParameter(parameters);

		for (const rapidxml::xml_node<utf8_t> *shapesNode = root.first_node("shapes");
			shapesNode; shapesNode = shapesNode->next_sibling("shapes"))
			for (const rapidxml::xml_node<utf8_t> *shapeNode = shapesNode->first_node();
				shapeNode; shapeNode = shapeNode->next_sibling())
			{
				utf8_ntr name = lean::get_attribute(*shapeNode, "name");
				utf8_ntr file = lean::get_attribute(*shapeNode, "file");

				// Do not overwrite shapes, if not permitted
				if (!bNoOverwrite || !shapeCache.GetByName(name))
				{
					lean::resource_ptr<AssembledShape> shape = shapeCache.GetByFile(file);
					try {
						shapeCache.SetName(shape, name);
					}
					catch (const bec::ResourceCollision<AssembledShape> &e)
					{
						LEAN_ASSERT(!bNoOverwrite);
						shapeCache.Replace(e.Resource, shape);
					}
				}
			}
	}
};


/// Loads a list of shapes.
class ShapeLoader : public beCore::LoadJob
{
public:
	/// Loads anything, e.g. to the given XML root node.
	void Load(const rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
		RigidShapeCache &rigidShapeCache = *LEAN_ASSERT_NOT_NULL(physicsParameters.ResourceManager)->RigidShapeCache();
		ShapeCache &shapeCache = *LEAN_ASSERT_NOT_NULL(physicsParameters.ResourceManager)->ShapeCache();
		MaterialCache &materialCache = *LEAN_ASSERT_NOT_NULL(physicsParameters.ResourceManager)->MaterialCache();

		bool bNoOverwrite = beEntitySystem::GetNoOverwriteParameter(parameters);

		for (const rapidxml::xml_node<utf8_t> *shapesNode = root.first_node("rigidshapes");
			shapesNode; shapesNode = shapesNode->next_sibling("rigidshapes"))
			for (const rapidxml::xml_node<utf8_t> *shapeNode = shapesNode->first_node();
				shapeNode; shapeNode = shapeNode->next_sibling())
			{
				utf8_ntr name = lean::get_attribute(*shapeNode, "name");

				// Do not overwrite shapes, if not permitted
				if (!bNoOverwrite || !rigidShapeCache.GetByName(name))
				{
					lean::resource_ptr<RigidShape> shape = LoadShape(*shapeNode, shapeCache, materialCache);
					try {
						rigidShapeCache.SetName(shape, name);
					}
					catch (const bec::ResourceCollision<RigidShape> &e)
					{
						LEAN_ASSERT(!bNoOverwrite);
						rigidShapeCache.Replace(e.Resource, shape);
					}
				}
			}
	}
};

} // namespace

const bec::LoadJob *CreateShapeImportLoader() { return new ShapeImportLoader(); }
const bec::LoadJob *CreateShapeLoader() { return new ShapeLoader(); }

} // namespace
