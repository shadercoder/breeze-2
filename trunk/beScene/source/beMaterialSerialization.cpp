/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMaterialSerialization.h"

#include <beGraphics/beEffectCache.h>
#include <beGraphics/beSetupSerialization.h>

#include <lean/xml/xml_file.h>
#include <lean/xml/utility.h>
#include <lean/xml/numeric.h>

#include <lean/smart/scoped_ptr.h>
#include <lean/containers/dynamic_array.h>

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace beScene
{

// Saves the given material to the given XML node.
void SaveMaterial(const Material &material, rapidxml::xml_node<lean::utf8_t> &node)
{
	const beGraphics::Effect *mainEffect = material.GetEffect();
	SaveEffect(*mainEffect, node);

	const uint4 setupCount = material.GetSetupCount();
	const uint4 techniqueCount = material.GetTechniqueCount();

	// Save setups
	for (uint4 setupIdx = 0; setupIdx < setupCount; ++setupIdx)
	{
		rapidxml::xml_node<utf8_t> &setupNode = *lean::allocate_node<utf8_t>(*node.document(), "setup");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&setupNode);

		const beGraphics::Setup &setup = *material.GetSetup(setupIdx);
		const beGraphics::Effect *effect = setup.GetEffect();

		// Only save effect if different from main material effect
		if (effect != mainEffect)
			SaveEffect(*effect, setupNode);

		SaveSetup(setup, setupNode);
	}

	// Save technique mapping
	for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
	{
		rapidxml::xml_node<utf8_t> &techniqueNode = *lean::allocate_node<utf8_t>(*node.document(), "technique");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&techniqueNode);

		utf8_ntr techniqueName = material.GetTechniqueName(techniqueIdx);
		const beGraphics::Setup *setup = material.GetTechniqueSetup(techniqueIdx);

		// Allow for robust re-mapping on load
		if (!techniqueName.empty())
			lean::append_attribute(*techniqueNode.document(), techniqueNode, "name", material.GetTechniqueName(techniqueIdx));
		lean::append_int_attribute(*techniqueNode.document(), techniqueNode, "idx", techniqueIdx);

		// Find setup index
		for (uint4 setupIdx = 0; setupIdx < setupCount; ++setupIdx)
			if (setup == material.GetSetup(setupIdx))
			{
				lean::append_int_attribute(*techniqueNode.document(), techniqueNode, "setup", setupIdx);
				break;
			}
	}
}

// Saves the given material to the given XML node.
void SaveMaterial(const Material &material, const utf8_ntri &file)
{
	lean::xml_file<lean::utf8_t> xml;
	rapidxml::xml_node<lean::utf8_t> &root = *lean::allocate_node<utf8_t>(xml.document(), "material");

	// ORDER: Append FIRST, otherwise parent document == nullptr
	xml.document().append_node(&root);
	SaveMaterial(material, root);

	xml.save(file);
}

// Load the given material from the given XML node.
lean::resource_ptr<Material, true> LoadMaterial(const rapidxml::xml_node<lean::utf8_t> &node,
	beGraphics::EffectCache &effects, beGraphics::TextureCache &textures, MaterialCache *pCache)
{
	lean::resource_ptr<Material> material = lean::new_resource<Material>(
			LoadEffect(node, effects),
			effects, textures, pCache
		);

	const uint4 count = material->GetSetupCount();
	const uint4 techniqueCount = material->GetTechniqueCount();

	const beGraphics::Effect &mainEffect = *material->GetEffect();
	bool bLossy = false;

	for (uint4 setupIdx = 0; setupIdx < count; ++setupIdx)
	{
		beGraphics::Setup &setup = *material->GetSetup(setupIdx);
		const beGraphics::Effect &effect = *setup.GetEffect();

		bool bLoaded = false;

		// Load setups by effect
		for (const rapidxml::xml_node<utf8_t> *srcSetupNode = node.first_node("setup");
			srcSetupNode; srcSetupNode = srcSetupNode->next_sibling("setup"))
		{
			// Identify corresponding effect
			const beGraphics::Effect *pSrcEffect = beGraphics::HasEffect(*srcSetupNode)
				? IdentifyEffect(*srcSetupNode, effects)
				: &mainEffect;

			if (&effect == pSrcEffect)
			{
				LoadSetup(setup, *srcSetupNode, textures);
				bLoaded = true;
				break;
			}
		}

		if (bLoaded)
			continue;
		else
			// Anything that follows is speculative
			bLossy = true;

		// Load setups by technique name
		for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
		{
			if (&setup == material->GetTechniqueSetup(techniqueIdx))
			{
				utf8_ntr techniqueName = material->GetTechniqueName(techniqueIdx);

				for (const rapidxml::xml_node<utf8_t> *srcTechniqueNode = node.first_node("technique");
					srcTechniqueNode; srcTechniqueNode = srcTechniqueNode->next_sibling("technique"))
				{
					utf8_ntr srcTechniqueName = lean::get_attribute(*srcTechniqueNode, "name");

					if (srcTechniqueName == techniqueName)
					{
						uint4 srcSetupIdx = lean::get_int_attribute(*srcTechniqueNode, "setup", 0);
						const rapidxml::xml_node<utf8_t> *pSrcSetupNode = lean::nth_child(node, srcSetupIdx, "setup");

						if (pSrcSetupNode)
						{
							LoadSetup(setup, *pSrcSetupNode, textures);
							bLoaded = true;
							break;
						}
					}
				}

				if (bLoaded)
					break;
			}
		}

		if (bLoaded)
			continue;

		// Load setups by technique index
		for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
		{
			if (&setup == material->GetTechniqueSetup(techniqueIdx))
			{
				for (const rapidxml::xml_node<utf8_t> *srcTechniqueNode = node.first_node("technique");
					srcTechniqueNode; srcTechniqueNode = srcTechniqueNode->next_sibling("technique"))
				{
					uint4 srcTechniqueIdx = lean::get_int_attribute(*srcTechniqueNode, "idx", static_cast<uint4>(-1));

					if (srcTechniqueIdx == techniqueIdx)
					{
						uint4 srcSetupIdx = lean::get_int_attribute(*srcTechniqueNode, "setup", 0);
						const rapidxml::xml_node<utf8_t> *pSrcSetupNode = lean::nth_child(node, srcSetupIdx, "setup");

						if (pSrcSetupNode)
						{
							LoadSetup(setup, *pSrcSetupNode, textures);
							bLoaded = true;
							break;
						}
					}
				}

				if (bLoaded)
					break;
			}
		}

		// bLoaded == false => nothing loaded (== keep defaults)
	}

	if (bLossy)
		LEAN_LOG_ERROR_MSG("Not all setups could be matched, some information might be lost");

	return material.transfer();
}

// Load the given material from the given XML node.
lean::resource_ptr<Material, true> LoadMaterial(const rapidxml::xml_document<lean::utf8_t> &document,
	beGraphics::EffectCache &effects, beGraphics::TextureCache &textures, MaterialCache *pCache)
{
	const rapidxml::xml_node<lean::utf8_t> *root = document.first_node("material");

	if (root)
		return LoadMaterial(*root, effects, textures, pCache);
	else
		LEAN_THROW_ERROR_MSG("Material root node missing");
}

// Load the given material from the given XML node.
lean::resource_ptr<Material, true> LoadMaterial(const utf8_ntri &file,
	beGraphics::EffectCache &effects, beGraphics::TextureCache &textures, MaterialCache *pCache)
{
	lean::resource_ptr<Material> pMaterial;

	LEAN_LOG("Attempting to load material \"" << file.c_str() << "\"");

	pMaterial = LoadMaterial( lean::xml_file<lean::utf8_t>(file).document(), effects, textures, pCache );

	LEAN_LOG("Material \"" << file.c_str() << "\" created successfully");

	return pMaterial.transfer();
}

} // namespace
