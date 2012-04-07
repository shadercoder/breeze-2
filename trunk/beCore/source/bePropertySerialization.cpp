/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/bePropertySerialization.h"
#include "beCore/beTextSerialization.h"

#include "beCore/bePropertyVisitor.h"

#include <lean/functional/predicates.h>
#include <lean/xml/utility.h>
#include <sstream>

#include <lean/logging/log.h>

namespace beCore
{

namespace
{

/// Property serializer.
struct PropertySerializer : public PropertyVisitor
{
	const PropertyProvider *properties;
	rapidxml::xml_node<lean::utf8_t> *parent;

	/// Constructor.
	PropertySerializer(const PropertyProvider &properties, rapidxml::xml_node<lean::utf8_t> &parent)
		: properties(&properties),
		parent(&parent) { }

	/// Visits the given values.
	void Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, const void *values)
	{
		rapidxml::xml_document<> &document = *LEAN_ASSERT_NOT_NULL(parent->document());
		
		const utf8_t *value = nullptr;

		const TextSerializer *pSerializer = GetTextSerialization().GetSerializer(desc.TypeID);

		if (pSerializer)
		{
			static const size_t StackBufferSize = 2048;

			size_t maxLength = pSerializer->GetMaxLength(desc.Count);

			// Use stack to speed up small allocations
			// NOTE: 0 means unpredictable
			if (maxLength != 0 && maxLength < StackBufferSize)
			{
				utf8_t buffer[StackBufferSize];
				
				// Serialize
				// NOTE: Required to be null-terminated -> xml
				utf8_t *bufferEnd = pSerializer->Write(buffer, desc.TypeInfo->type, values, desc.Count);
				*bufferEnd++ = 0;

				size_t length = bufferEnd - buffer;
				LEAN_ASSERT(length < StackBufferSize);
				
				value = document.allocate_string(buffer, length);
			}
			// Take generic route, otherwise
			else
			{
				std::basic_ostringstream<utf8_t> stream;
				stream.imbue(std::locale::classic());

				// Serialize
				pSerializer->Write(stream, desc.TypeInfo->type, values, desc.Count);

				value = document.allocate_string( stream.str().c_str() );
			}
		}
		else
			LEAN_LOG_ERROR("No serializer available for type \"" << GetTypeIndex().GetName(desc.TypeID) << " (" << desc.TypeID << ")");

		rapidxml::xml_node<utf8_t> &node = *lean::allocate_node(document, "p", value);
		node.append_attribute(
				lean::allocate_attribute(document, "n", properties->GetPropertyName(propertyID))
			);
		parent->append_node(&node);
	}
};

/// Property deserializer.
struct PropertyDeserializer : public PropertyVisitor
{
	PropertyProvider *properties;
	const rapidxml::xml_node<lean::utf8_t> *node;

	/// Constructor.
	PropertyDeserializer(PropertyProvider &properties, const rapidxml::xml_node<lean::utf8_t> &node)
		: properties(&properties),
		node(&node) { }

	/// Visits the given values.
	bool Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, void *values)
	{
		const TextSerializer *pSerializer = GetTextSerialization().GetSerializer(desc.TypeID);

		if (pSerializer)
		{
			if (pSerializer->Read(node->value(), node->value() + node->value_size(), desc.TypeInfo->type, values, desc.Count))
				return true;
			// TODO: error logging?
		}
		else
			LEAN_LOG_ERROR("No serializer available for type \"" << GetTypeIndex().GetName(desc.TypeID) << " (" << desc.TypeID << ")");

		return false;
	}
};

} // namespace

// Saves the given property provider to the given XML node.
void SaveProperties(const PropertyProvider &properties, rapidxml::xml_node<lean::utf8_t> &node, bool bPersistentOnly)
{
	const uint4 propertyCount = properties.GetPropertyCount();

	if (propertyCount > 0)
	{
		rapidxml::xml_document<utf8_t> &document = *node.document();

		rapidxml::xml_node<utf8_t> &propertiesNode = *lean::allocate_node<utf8_t>(document, "properties");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&propertiesNode);

		PropertySerializer serializer(properties, propertiesNode);

		for (uint4 i = 0; i < propertyCount; ++i)
			properties.ReadProperty(i, serializer, bPersistentOnly);
	}
}

// Saves the given property provider to the given XML node.
void SaveProperties(const PropertyProvider &properties, uint4 propertyID, rapidxml::xml_node<lean::utf8_t> &node, bool bPersistentOnly)
{
	PropertySerializer serializer(properties, node);
	properties.ReadProperty(propertyID, serializer, bPersistentOnly);
}

// Load the given property provider from the given XML node.
void LoadProperties(PropertyProvider &properties, const rapidxml::xml_node<lean::utf8_t> &node)
{
	const uint4 propertyCount = properties.GetPropertyCount();

	uint4 nextPropertyID = 0;

	for (const rapidxml::xml_node<lean::utf8_t> *propertiesNode = node.first_node("properties");
		propertiesNode; propertiesNode = propertiesNode->next_sibling("properties"))
		for (const rapidxml::xml_node<lean::utf8_t> *propertyNode = propertiesNode->first_node();
			propertyNode; propertyNode = propertyNode->next_sibling())
		{
			const utf8_t *nodeName = propertyNode->first_attribute() ? propertyNode->first_attribute()->value() : propertyNode->name();

			uint4 lowerPropertyID = nextPropertyID;
			uint4 upperPropertyID = nextPropertyID;

			for (uint4 i = 0; i < propertyCount; ++i)
			{
				// Perform bi-directional search: even == forward; odd == backward
				uint4 propertyID = (lean::is_odd(i) | (upperPropertyID == propertyCount)) & (lowerPropertyID != 0)
					? --lowerPropertyID
					: upperPropertyID++;

				if (nodeName == properties.GetPropertyName(propertyID))
				{
					PropertyDeserializer serializer(properties, *propertyNode);
					properties.WriteProperty(propertyID, serializer);

					// Start next search with next property
					nextPropertyID = propertyID + 1;
					break;
				}
			}
		}
}

// Load the given property provider from the given XML node.
void LoadProperties(PropertyProvider &properties, uint4 propertyID, const rapidxml::xml_node<lean::utf8_t> &node)
{
	utf8_ntr propertyName = properties.GetPropertyName(propertyID);

	for (const rapidxml::xml_node<lean::utf8_t> *propertiesNode = node.first_node("properties");
		propertiesNode; propertiesNode = propertiesNode->next_sibling("properties"))
		for (const rapidxml::xml_node<lean::utf8_t> *propertyNode = propertiesNode->first_node();
			propertyNode; propertyNode = propertyNode->next_sibling())
		{
			const utf8_t *nodeName = propertyNode->first_attribute() ? propertyNode->first_attribute()->value() : propertyNode->name();

			if (nodeName == propertyName)
			{
				PropertyDeserializer serializer(properties, *propertyNode);
				properties.WriteProperty(propertyID, serializer);
			}
		}
}

} // namespace
