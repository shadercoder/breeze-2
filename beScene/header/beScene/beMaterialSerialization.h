/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MATERIAL_SERIALIZATION
#define BE_SCENE_MATERIAL_SERIALIZATION

#include "beScene.h"
#include "beMaterial.h"
#include <lean/rapidxml/rapidxml.hpp>
#include <lean/smart/resource_ptr.h>

#include "beMaterialCache.h"

namespace beScene
{

/// Saves the given material to the given XML node.
BE_SCENE_API void SaveMaterial(const Material &material, rapidxml::xml_node<lean::utf8_t> &node);
/// Saves the given material to the given XML node.
BE_SCENE_API void SaveMaterial(const Material &material, const utf8_ntri &file);

/// Load the given material from the given XML node.
BE_SCENE_API lean::resource_ptr<Material, true> LoadMaterial(const rapidxml::xml_node<lean::utf8_t> &node,
	beGraphics::EffectCache &effects, beGraphics::TextureCache &textures, MaterialCache *pCache = nullptr);
/// Load the given material from the given XML node.
BE_SCENE_API lean::resource_ptr<Material, true> LoadMaterial(const rapidxml::xml_document<lean::utf8_t> &document,
	beGraphics::EffectCache &effects, beGraphics::TextureCache &textures, MaterialCache *pCache = nullptr);
/// Load the given material from the given XML node.
BE_SCENE_API lean::resource_ptr<Material, true> LoadMaterial(const utf8_ntri &file, beGraphics::EffectCache &effects,
	beGraphics::TextureCache &textures, MaterialCache *pCache = nullptr);

} // namespace

#endif