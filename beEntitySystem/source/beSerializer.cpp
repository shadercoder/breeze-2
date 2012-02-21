/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beSerializer.h"
#include <lean/xml/utility.h>
#include <lean/xml/numeric.h>

namespace beEntitySystem
{

// Constructor.
template <class Serializable>
Serializer<Serializable>::Serializer(const utf8_ntri &tye)
	: m_type(tye.to<utf8_string>())
{
}

// Destructor.
template <class Serializable>
Serializer<Serializable>::~Serializer()
{
}

// Gets the name of the serializable object stored in the given xml node.
template <class Serializable>
utf8_ntr Serializer<Serializable>::GetName(const rapidxml::xml_node<lean::utf8_t> &node)
{
	return lean::get_attribute(node, "name");
}

/// Sets the name of the serializable object stored in the given xml node.
template <class Serializable>
void Serializer<Serializable>::SetName(const utf8_ntri &name, rapidxml::xml_node<lean::utf8_t> &node)
{
	lean::append_attribute(*node.document(), node, "name", name);
}

// Gets the type of the serializable object stored in the given xml node.
template <class Serializable>
utf8_ntr Serializer<Serializable>::GetType(const rapidxml::xml_node<lean::utf8_t> &node)
{
	return lean::get_attribute(node, "type");
}

// Gets the ID of the serializable object stored in the given xml node.
template <class Serializable>
uint8 Serializer<Serializable>::GetID(const rapidxml::xml_node<lean::utf8_t> &node)
{
	return lean::get_int_attribute(node, "id", static_cast<uint8>(-1));
}

// Sets the ID of the serializable object stored in the given xml node.
template <class Serializable>
void Serializer<Serializable>::SetID(uint8 id, rapidxml::xml_node<lean::utf8_t> &node)
{
	lean::append_int_attribute(*node.document(), node, "id", id);
}

// Gets a list of creation parameters.
template <class Serializable>
typename Serializer<Serializable>::SerializationParameters Serializer<Serializable>::GetCreationParameters() const
{
	return SerializationParameters(nullptr, nullptr);
}

// Loads a serializable object from the given xml node.
template <class Serializable>
void Serializer<Serializable>::Load(Serializable *pSerializable, const rapidxml::xml_node<lean::utf8_t> &node, 
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
}

// Saves the given serializable object to the given XML node.
template <class Serializable>
void Serializer<Serializable>::Save(const Serializable *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const
{
	rapidxml::xml_document<lean::utf8_t> &document = *node.document();

	lean::append_attribute(document, node, "type", pSerializable->GetType());
}

} // namespace
