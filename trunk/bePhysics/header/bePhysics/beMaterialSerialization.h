/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_MATERIAL_SERIALIZATION
#define BE_PHYSICS_MATERIAL_SERIALIZATION

#include "bePhysics.h"
#include "beMaterial.h"
#include <lean/rapidxml/rapidxml.hpp>

namespace bePhysics
{

/// Load the given material from the given XML node.
BE_PHYSICS_API void LoadMaterial(Material &material, const rapidxml::xml_node<lean::utf8_t> &node);
/// Loads the given material from the given XML file.
BE_PHYSICS_API void LoadMaterial(Material &material, const utf8_ntri &file);

/// Loads the given material from the given XML node.
BE_PHYSICS_API lean::resource_ptr<Material, true> LoadMaterial(Device &device, const rapidxml::xml_node<lean::utf8_t> &node, MaterialCache *pMaterialCache = nullptr);
/// Loads the given material from the given XML file.
BE_PHYSICS_API lean::resource_ptr<Material, true> LoadMaterial(Device &device, const utf8_ntri &file, MaterialCache *pMaterialCache = nullptr);

/// Saves the given material to the given XML node.
BE_PHYSICS_API void SaveMaterial(const Material &material, rapidxml::xml_node<lean::utf8_t> &node);
/// Saves the given material to the given XML file.
BE_PHYSICS_API void SaveMaterial(const Material &material, const utf8_ntri &file);

} // namespace

#endif