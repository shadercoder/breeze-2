/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_PROPERTY_SERIALIZATION
#define BE_CORE_PROPERTY_SERIALIZATION

#include "beCore.h"
#include "bePropertyProvider.h"
#include <lean/rapidxml/rapidxml.hpp>

namespace beCore
{

/// Saves the given property provider to the given XML node.
BE_CORE_API void SaveProperties(const PropertyProvider &properties, rapidxml::xml_node<lean::utf8_t> &node, bool bPersistentOnly = true);
/// Saves the given property provider to the given XML node.
BE_CORE_API void SaveProperties(const PropertyProvider &properties, uint4 propertyID, rapidxml::xml_node<lean::utf8_t> &node, bool bPersistentOnly = true);

/// Load the given property provider from the given XML node.
BE_CORE_API void LoadProperties(PropertyProvider &properties, const rapidxml::xml_node<lean::utf8_t> &node);
/// Load the given property provider from the given XML node.
BE_CORE_API void LoadProperties(PropertyProvider &properties, uint4 propertyID, const rapidxml::xml_node<lean::utf8_t> &node);

} // namespace

#endif