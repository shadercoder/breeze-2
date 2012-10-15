/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_SETUP_SERIALIZATION
#define BE_GRAPHICS_SETUP_SERIALIZATION

#include "beGraphics.h"
#include "beSetup.h"
#include <lean/rapidxml/rapidxml.hpp>

namespace beGraphics
{

class TextureCache;
class EffectCache;

/// Saves the given effect to the given XML node.
BE_GRAPHICS_API void SaveEffect(const Effect &effect, rapidxml::xml_node<lean::utf8_t> &node);
/// Loads an effect from the given XML node.
BE_GRAPHICS_API Effect* LoadEffect(const rapidxml::xml_node<lean::utf8_t> &node, EffectCache &cache);
/// Loads an effect from the given XML node.
BE_GRAPHICS_API Effect* IdentifyEffect(const rapidxml::xml_node<lean::utf8_t> &node, const EffectCache &cache);
/// Checks if the given XML node specifies an effect.
BE_GRAPHICS_API bool HasEffect(const rapidxml::xml_node<lean::utf8_t> &node);

/// Saves the textures provided by the given object to the given XML node.
BE_GRAPHICS_API void SaveTextures(const TextureProvider &textures, rapidxml::xml_node<lean::utf8_t> &node);
/// Loads the textures provided by the given object from the given XML node.
BE_GRAPHICS_API void LoadTextures(TextureProvider &textures, const rapidxml::xml_node<lean::utf8_t> &node, TextureCache &cache);

/// Saves the given setup to the given XML node.
BE_GRAPHICS_API void SaveSetup(const Setup &setup, rapidxml::xml_node<lean::utf8_t> &node);
/// Saves the given setup to the given XML file.
BE_GRAPHICS_API void SaveSetup(const Setup &setup, const utf8_ntri &file);

/// Load the given setup from the given XML node.
BE_GRAPHICS_API void LoadSetup(Setup &setup, const rapidxml::xml_node<lean::utf8_t> &node, TextureCache &textureCache);
/// Load the given setup from the given XML document.
BE_GRAPHICS_API void LoadSetup(Setup &setup, const rapidxml::xml_document<lean::utf8_t> &document, TextureCache &textureCache);
/// Load the given setup from the given XML file.
BE_GRAPHICS_API void LoadSetup(Setup &setup, const utf8_ntri &file, TextureCache &textureCache);

} // namespace

#endif