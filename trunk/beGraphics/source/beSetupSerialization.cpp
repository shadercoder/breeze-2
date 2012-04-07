/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/beSetupSerialization.h"

#include "beGraphics/beTextureCache.h"
#include "beGraphics/beEffectCache.h"

#include <beCore/bePropertySerialization.h>

#include <lean/xml/numeric.h>
#include <lean/xml/xml_file.h>
#include <lean/functional/predicates.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beGraphics
{

// Saves the given effect to the given XML node.
void SaveEffect(const Effect &effect, rapidxml::xml_node<lean::utf8_t> &node)
{
	EffectCache *pCache = effect.GetCache();

	if (pCache)
	{
		beCore::Exchange::utf8_string macroString;
		utf8_ntr file = pCache->GetFile(effect, &macroString);

		if (!file.empty())
		{
			lean::append_attribute(*node.document(), node, "effect", pCache->GetPathResolver().Shorten(file));

			if (!macroString.empty())
				lean::append_attribute(*node.document(), node, "effectOptions", macroString);
		}
		else
			LEAN_LOG_ERROR_CTX("Could not identify effect, will be lost", node.name());
	}
	else
		LEAN_LOG_ERROR_CTX("Could not identify effect cache, effect will be lost", node.name());
}

// Loads an effect from the given XML node.
Effect* LoadEffect(const rapidxml::xml_node<lean::utf8_t> &node, EffectCache &cache)
{
	utf8_ntr effectFile = lean::get_attribute(node, "effect");
	utf8_ntr effectMacros = lean::get_attribute(node, "effectOptions");

	if (!effectFile.empty())
		return cache.GetEffect(effectFile, effectMacros);
	else
		LEAN_THROW_ERROR_CTX("Node is missing a valid effect specification", node.name());
}

// Loads an effect from the given XML node.
Effect* IdentifyEffect(const rapidxml::xml_node<lean::utf8_t> &node, const EffectCache &cache)
{
	utf8_ntr effectFile = lean::get_attribute(node, "effect");
	utf8_ntr effectMacros = lean::get_attribute(node, "effectOptions");

	if (!effectFile.empty())
		return cache.IdentifyEffect(effectFile, effectMacros);
	else
		LEAN_THROW_ERROR_CTX("Node is missing a valid effect specification", node.name());
}

// Checks if the given XML node specifies an effect.
bool HasEffect(const rapidxml::xml_node<lean::utf8_t> &node)
{
	return !lean::get_attribute(node, "effect").empty();
}

// Saves the textures provided by the given object to the given XML node.
void SaveTextures(const TextureProvider &textures, rapidxml::xml_node<lean::utf8_t> &node)
{
	const uint4 textureCount = textures.GetTextureCount();

	if (textureCount > 0)
	{
		rapidxml::xml_document<utf8_t> &document = *node.document();

		rapidxml::xml_node<utf8_t> &texturesNode = *lean::allocate_node<utf8_t>(document, "textures");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&texturesNode);

		for (uint4 i = 0; i < textureCount; ++i)
		{
			utf8_ntr textureName = textures.GetTextureName(i);

			rapidxml::xml_node<utf8_t> &textureNode = *lean::allocate_node(document, "t");
			// ORDER: Append FIRST, otherwise parent document == nullptrs
			texturesNode.append_node(&textureNode);

			textureNode.append_attribute(
					lean::allocate_attribute(document, "n", textureName)
				);

			const TextureView *pTexture = textures.GetTexture(i);
			TextureCache *pCache = (pTexture) ? pTexture->GetCache() : nullptr;

			if (pCache)
			{
				bool bIsFile = false;
				utf8_ntr file = pCache->GetFile(*pTexture, &bIsFile);

				if (bIsFile)
					lean::append_attribute(document, textureNode, "file", pCache->GetPathResolver().Shorten(file));
				else if (!file.empty())
					lean::append_attribute(document, textureNode, "name", file);
				else
					LEAN_LOG_ERROR_CTX("Could not identify texture, will be lost", textureName.c_str());
			}
			else
				LEAN_LOG_ERROR_CTX("Could not identify texture cache, texture will be lost", textureName.c_str());
		}
	}
}

// Loads the textures provided by the given object from the given XML node.
void LoadTextures(TextureProvider &textures, const rapidxml::xml_node<lean::utf8_t> &node, TextureCache &cache)
{
	const uint4 textureCount = textures.GetTextureCount();

	uint4 nextTextureID = 0;

	for (const rapidxml::xml_node<lean::utf8_t> *texturesNode = node.first_node("textures");
		texturesNode; texturesNode = texturesNode->next_sibling("textures"))
		for (const rapidxml::xml_node<lean::utf8_t> *textureNode = texturesNode->first_node();
			textureNode; textureNode = textureNode->next_sibling())
		{
			utf8_ntr textureName = lean::get_attribute(*textureNode, "n");

			if (textureName.empty())
				textureName = utf8_ntr( textureNode->name() );

			utf8_ntr file = lean::get_attribute(*textureNode, "file");
			utf8_ntr name = lean::get_attribute(*textureNode, "name");

			uint4 lowerTextureID = nextTextureID;
			uint4 upperTextureID = nextTextureID;

			if (!file.empty() || !name.empty())
				for (uint4 i = 0; i < textureCount; ++i)
				{
					// Perform bi-directional search: even == forward; odd == backward
					uint4 textureID = (lean::is_odd(i) | (upperTextureID == textureCount)) & (lowerTextureID != 0)
						? --lowerTextureID
						: upperTextureID++;

					if (textureName == textures.GetTextureName(textureID))
					{
						if (file.empty())
							// TODO: Remove
							LEAN_LOG_ERROR_XCTX("TODO: Named textures not supported yet", name.c_str(), textureName.c_str() );
						else
						{
							TextureView *pTexture = // (!file.empty())
								/*?*/ cache.GetTextureView(file);
//								: nullptr; // TODO: cache.GetTextureViewByName(name, true);

							textures.SetTexture(textureID, pTexture);
						}

						// Start next search with next texture
						nextTextureID = textureID + 1;
						break;
					}
				}
			else
				LEAN_LOG_ERROR_CTX("Texture is missing both file and name specification", textureName.c_str());
		}
}

// Saves the given setup to the given XML node.
void SaveSetup(const Setup &setup, rapidxml::xml_node<lean::utf8_t> &node)
{
	SaveProperties(setup, node);
	SaveTextures(setup, node);
}

// Saves the given setup to the given XML node.
void SaveSetup(const Setup &setup, const utf8_ntri &file)
{
	lean::xml_file<lean::utf8_t> xml;
	rapidxml::xml_node<lean::utf8_t> &root = *lean::allocate_node<utf8_t>(xml.document(), "setup");

	// ORDER: Append FIRST, otherwise parent document == nullptr
	xml.document().append_node(&root);
	SaveSetup(setup, root);

	xml.save(file);
}

// Load the given setup from the given XML node.
void LoadSetup(Setup &setup, const rapidxml::xml_node<lean::utf8_t> &node, TextureCache &textureCache)
{
	LoadProperties(setup, node);
	LoadTextures(setup, node, textureCache);
}

// Load the given setup from the given XML node.
void LoadSetup(Setup &setup, const rapidxml::xml_document<lean::utf8_t> &document, TextureCache &textureCache)
{
	const rapidxml::xml_node<lean::utf8_t> *root = document.first_node("setup");

	if (root)
		return LoadSetup(setup, *root, textureCache);
	else
		LEAN_THROW_ERROR_MSG("Setup root node missing");
}

// Load the given setup from the given XML node.
void LoadSetup(Setup &setup, const utf8_ntri &file, TextureCache &textureCache)
{
	LEAN_LOG("Attempting to load setup \"" << file.c_str() << "\"");

	LoadSetup( setup, lean::xml_file<lean::utf8_t>(file).document(), textureCache );

	LEAN_LOG("setup \"" << file.c_str() << "\" created successfully");
}

} // namespace
