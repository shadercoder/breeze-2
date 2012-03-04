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
	const beGraphics::Effect *pMainEffect = material.GetEffect();
	SaveEffect(*pMainEffect, node);

	const uint4 techniqueCount = material.GetTechniqueCount();
	
	typedef lean::dynamic_array<beGraphics::Setup*> setup_array;
	typedef lean::dynamic_array<uint4> idx_array;
	idx_array setupIndices(techniqueCount);
	setup_array setups(techniqueCount);

	// Collect unique setups
	for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
	{
		beGraphics::Setup *pSetup = material.GetSetup(techniqueIdx);
		uint4 setupIdx = techniqueCount;

		// Find shared setups
		for (setup_array::const_iterator it = setups.begin(); it != setups.end(); ++it)
			if (*it == pSetup)
			{
				setupIdx = static_cast<uint4>( it - setups.begin() );
				break;
			}

		if (setupIdx == techniqueCount)
		{
			setupIdx = static_cast<uint4>( setups.size() );
			setups.push_back(pSetup);
		}

		setupIndices.push_back(setupIdx);
	}

	// Save setups
	for (setup_array::const_iterator it = setups.begin(); it != setups.end(); ++it)
	{
		rapidxml::xml_node<utf8_t> &setupNode = *lean::allocate_node<utf8_t>(*node.document(), "setup");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&setupNode);

		const beGraphics::Effect *pEffect = (*it)->GetEffect();

		// Only save effect if different from main material effect
		if (pEffect != pMainEffect)
			SaveEffect(*pEffect, setupNode);

		SaveSetup(**it, setupNode);
	}

	// Save technique mapping
	for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
	{
		rapidxml::xml_node<utf8_t> &techniqueNode = *lean::allocate_node<utf8_t>(*node.document(), "technique");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&techniqueNode);

		utf8_ntr techniqueName = material.GetTechniqueName(techniqueIdx);

		// Allow for robust re-mapping on load
		if (!techniqueName.empty())
			lean::append_attribute(*techniqueNode.document(), techniqueNode, "name", material.GetTechniqueName(techniqueIdx));
		lean::append_int_attribute(*techniqueNode.document(), techniqueNode, "idx", techniqueIdx);

		lean::append_int_attribute(*techniqueNode.document(), techniqueNode, "setup", setupIndices[techniqueIdx]);
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
	lean::resource_ptr<Material> pMaterial;

	pMaterial = lean::new_resource<Material>(
			LoadEffect(node, effects),
			effects, textures, pCache
		);

	const beGraphics::Effect *pMainEffect = pMaterial->GetEffect();
	const uint4 techniqueCount = pMaterial->GetTechniqueCount();

	// TODO: Move to material itself (already has setup index)
	typedef lean::dynamic_array<beGraphics::Setup*> setup_array;
	typedef lean::dynamic_array<uint4> idx_array;
	idx_array setupIndices(techniqueCount);
	setup_array setups(techniqueCount);

	// Collect unique setups
	for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
	{
		beGraphics::Setup *pSetup = pMaterial->GetSetup(techniqueIdx);
		uint4 setupIdx = techniqueCount;

		// Find shared setups
		for (setup_array::const_iterator it = setups.begin(); it != setups.end(); ++it)
			if (*it == pSetup)
			{
				setupIdx = static_cast<uint4>( it - setups.begin() );
				break;
			}

		if (setupIdx == techniqueCount)
		{
			setupIdx = static_cast<uint4>( setups.size() );
			setups.push_back(pSetup);
		}

		setupIndices.push_back(setupIdx);
	}

	typedef lean::dynamic_array<const rapidxml::xml_node<utf8_t>*> node_array;
	node_array setupNodes( lean::node_count(node, "setup") );

	uint4 setupLoadCount = 0;

	// Load setups by effect
	for (const rapidxml::xml_node<utf8_t> *setupNode = node.first_node("setup");
		setupNode; setupNode = setupNode->next_sibling("setup"))
	{
		setupNodes.push_back(setupNode);

		if (!beGraphics::HasEffect(*setupNode))
		{
			// Load main setup (skips explicit effect identification)
			for (setup_array::iterator itSetup = setups.begin(); itSetup != setups.end(); ++itSetup)
			{
				beGraphics::Setup *&pSetup = *itSetup;

				if (pSetup && pSetup->GetEffect() == pMainEffect)
				{
					LoadSetup(*pSetup, *setupNode, textures);
					// Mark as loaded
					pSetup = nullptr;
					++setupLoadCount;
				}
			}
		}
		else
		{
			// Identify corresponding effect
			const beGraphics::Effect *pSetupEffect = IdentifyEffect(*setupNode, effects);

			// Apply effect-specific setups
			if (pSetupEffect)
				for (setup_array::iterator itSetup = setups.begin(); itSetup != setups.end(); ++itSetup)
				{
					beGraphics::Setup *&pSetup = *itSetup;

					if (pSetup && pSetup->GetEffect() == pSetupEffect)
					{
						LoadSetup(*pSetup, *setupNode, textures);
						// Mark as loaded
						pSetup = nullptr;
						++setupLoadCount;
					}
				}
		}
	}

	if (setupLoadCount < setups.size())
	{
		// Load techniques by name
		for (const rapidxml::xml_node<utf8_t> *techniqueNode = node.first_node();
			techniqueNode; techniqueNode = techniqueNode->next_sibling())
		{
			utf8_ntr techniqueName = lean::get_attribute(*techniqueNode, "name");
		
			if (!techniqueName.empty())
			{
				uint4 setupIdx = lean::get_int_attribute(*techniqueNode, "setup", static_cast<uint4>(-1));

				if (setupIdx < setupNodes.size())
				{
					// Find matching technique names
					for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
						if (pMaterial->GetTechniqueName(techniqueIdx) == techniqueName)
						{
							beGraphics::Setup *&pSetup = setups[setupIndices[techniqueIdx]];

							if (pSetup)
							{
								LoadSetup(*pSetup, *setupNodes[setupIdx], textures);
								// Mark as loaded
								pSetup = nullptr;
								++setupLoadCount;
							}
						}
				}
				else
					LEAN_LOG_ERROR_CTX("Technique setup index out of range", techniqueName.c_str());
			}
		}

		if (setupLoadCount < setups.size())
		{
			// Load techniques by index
			for (const rapidxml::xml_node<utf8_t> *techniqueNode = node.first_node();
				techniqueNode; techniqueNode = techniqueNode->next_sibling())
			{
				uint4 techniqueIdx = lean::get_int_attribute(*techniqueNode, "idx", static_cast<uint4>(-1));
		
				if (techniqueIdx < techniqueCount)
				{
					uint4 setupIdx = lean::get_int_attribute(*techniqueNode, "setup", static_cast<uint4>(-1));

					if (setupIdx < setupNodes.size())
					{
						beGraphics::Setup *&pSetup = setups[setupIndices[techniqueIdx]];

						if (pSetup)
						{
							LoadSetup(*pSetup, *setupNodes[setupIdx], textures);
							// Mark as loaded
							pSetup = nullptr;
							++setupLoadCount;
						}
					}
					else
						LEAN_LOG_ERROR_MSG("Technique setup index out of range");
				}
			}

			if (setupLoadCount < setups.size())
				LEAN_LOG_ERROR_MSG("Not all setups could be matched, some information might be lost");
		}
	}

	return pMaterial.transfer();
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
