/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beInlineMaterialSerialization.h"
#include "bePhysics/beMaterialSerialization.h"

#include <beEntitySystem/beSerializationParameters.h>
#include <beEntitySystem/beSerializationTasks.h>

#include "bePhysics/beSerializationParameters.h"
#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"

#include <lean/xml/utility.h>
#include <lean/xml/numeric.h>
#include <set>

namespace bePhysics
{

namespace
{

const uint4 MaterialSerializerID = beEntitySystem::GetSerializationParameters().Add("bePhysics.MaterialSerializer");

/// Serializes a list of materials.
class MaterialSerializer : public beEntitySystem::SaveJob
{
private:
	typedef std::set<const Material*> material_set;
	material_set m_materials;

public:
	/// Adds the given material for serialization.
	void AddMaterial(const Material *pMaterial)
	{
		LEAN_ASSERT_NOT_NULL( pMaterial );
		LEAN_ASSERT_NOT_NULL( pMaterial->GetCache() );
		m_materials.insert(pMaterial);
	}

	/// Saves anything, e.g. to the given XML root node.
	void Save(rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<SaveJob> &queue) const
	{
		rapidxml::xml_document<utf8_t> &document = *root.document();

		rapidxml::xml_node<utf8_t> &materialsNode = *lean::allocate_node<utf8_t>(document, "physicsmaterials");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		root.append_node(&materialsNode);

		for (material_set::const_iterator itMaterial = m_materials.begin();
			itMaterial != m_materials.end(); itMaterial++)
		{
			rapidxml::xml_node<utf8_t> &materialNode = *lean::allocate_node<utf8_t>(document, "m");
			// ORDER: Append FIRST, otherwise parent document == nullptr
			materialsNode.append_node(&materialNode);

			const Material *pMaterial = *itMaterial;

			lean::append_attribute( document, materialNode, "name", pMaterial->GetCache()->GetName(pMaterial) );
			SaveMaterial(*pMaterial, materialNode);
		}
	}
};

} // namespace

// Schedules the given material for inline serialization.
void SaveMaterial(const Material *pMaterial, beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::SaveJob> &queue)
{
	MaterialSerializer *pSerializer = parameters.GetValueDefault< MaterialSerializer* >(beEntitySystem::GetSerializationParameters(), MaterialSerializerID, nullptr);

	// Create serializer on first call
	if (!pSerializer)
	{
		// NOTE: Serialization queue takes ownership
		pSerializer = new MaterialSerializer();
		queue.AddSerializationJob(pSerializer);
		parameters.SetValue< MaterialSerializer* >(beEntitySystem::GetSerializationParameters(), MaterialSerializerID, pSerializer);
	}

	// Schedule material for serialization
	pSerializer->AddMaterial(pMaterial);
}


namespace
{

/// Loads a list of materials.
class MaterialLoader : public beEntitySystem::LoadJob
{
public:
	/// Loads anything, e.g. to the given XML root node.
	void Load(const rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<LoadJob> &queue) const
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
		MaterialCache &materialCache = *LEAN_ASSERT_NOT_NULL(physicsParameters.ResourceManager)->MaterialCache();

		bool bNoOverwrite = beEntitySystem::GetNoOverwriteParameter(parameters);

		for (const rapidxml::xml_node<utf8_t> *materialsNode = root.first_node("physicsmaterials");
			materialsNode; materialsNode = materialsNode->next_sibling("physicsmaterials"))
			for (const rapidxml::xml_node<utf8_t> *materialNode = materialsNode->first_node();
				materialNode; materialNode = materialNode->next_sibling())
			{
				utf8_ntr name = lean::get_attribute(*materialNode, "name");

				// Do not overwrite materials, if not permitted
				if (!bNoOverwrite || !materialCache.GetByName(name))
				{
					lean::resource_ptr<Material> pMaterial = LoadMaterial(*physicsParameters.Device, *materialNode, &materialCache);
					materialCache.Set(pMaterial, name);
				}
			}
	}
};

const beEntitySystem::LoadTaskPlugin<MaterialLoader, &beEntitySystem::GetResourceLoadTasks> MaterialLoaderPlugin;

} // namespace

} // namespace
