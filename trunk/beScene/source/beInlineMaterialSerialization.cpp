/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beInlineMaterialSerialization.h"
#include "beScene/beMaterialSerialization.h"

#include <beEntitySystem/beSerializationParameters.h>
#include <beEntitySystem/beSerializationTasks.h>

#include "beScene/beSerializationParameters.h"
#include "beScene/beResourceManager.h"
#include "beScene/beMaterialCache.h"

#include <lean/xml/utility.h>
#include <lean/xml/numeric.h>
#include <set>

namespace beScene
{

namespace
{

const uint4 MaterialSerializerID = beEntitySystem::GetSerializationParameters().Add("beScene.MaterialSerializer");

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

		rapidxml::xml_node<utf8_t> &materialsNode = *lean::allocate_node<utf8_t>(document, "materials");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		root.append_node(&materialsNode);

		for (material_set::const_iterator itMaterial = m_materials.begin();
			itMaterial != m_materials.end(); itMaterial++)
		{
			rapidxml::xml_node<utf8_t> &materialNode = *lean::allocate_node<utf8_t>(document, "m");
			// ORDER: Append FIRST, otherwise parent document == nullptr
			materialsNode.append_node(&materialNode);

			const Material *pMaterial = *itMaterial;

			lean::append_attribute( document, materialNode, "name", pMaterial->GetCache()->GetFile(pMaterial) );
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
		SceneParameters sceneParameters = GetSceneParameters(parameters);
		beGraphics::EffectCache &effectCache = *LEAN_ASSERT_NOT_NULL(sceneParameters.ResourceManager)->EffectCache();
		beGraphics::TextureCache &textureCache = *LEAN_ASSERT_NOT_NULL(sceneParameters.ResourceManager)->TextureCache();
		MaterialCache &materialCache = *LEAN_ASSERT_NOT_NULL(sceneParameters.ResourceManager)->MaterialCache();

		for (const rapidxml::xml_node<utf8_t> *materialsNode = root.first_node("materials");
			materialsNode; materialsNode = materialsNode->next_sibling("materials"))
			for (const rapidxml::xml_node<utf8_t> *materialNode = materialsNode->first_node();
				materialNode; materialNode = materialNode->next_sibling())
			{
				utf8_ntr name = lean::get_attribute(*materialNode, "name");

				lean::resource_ptr<Material> pMaterial = LoadMaterial(*materialNode, effectCache, textureCache, &materialCache);
				materialCache.SetMaterialName(name, pMaterial);
			}
	}
};

const beEntitySystem::LoadTaskPlugin<MaterialLoader> MaterialLoaderPlugin;

} // namespace

} // namespace
