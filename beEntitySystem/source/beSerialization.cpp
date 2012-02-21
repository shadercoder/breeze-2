/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beSerialization.h"
#include "beEntitySystem/beSerializer.h"
#include <lean/xml/utility.h>

namespace beEntitySystem
{

// Constructor.
template <class Serializable, class Serializer>
Serialization<Serializable, Serializer>::Serialization()
{
}

// Destructor.
template <class Serializable, class Serializer>
Serialization<Serializable, Serializer>::~Serialization()
{
}

// Adds the given serializer to this serialization manager.
template <class Serializable, class Serializer>
void Serialization<Serializable, Serializer>::AddSerializer(const Serializer *pSerializer)
{
	LEAN_ASSERT(pSerializer);

	m_serializers[pSerializer->GetType()] = pSerializer;
}

// Removes the given serializer from this serialization manager.
template <class Serializable, class Serializer>
bool Serialization<Serializable, Serializer>::RemoveSerializer(const Serializer *pSerializer)
{
	return (m_serializers.erase(pSerializer->GetType()) != 0);
}

// Gets the number of serializers.
template <class Serializable, class Serializer>
uint4 Serialization<Serializable, Serializer>::GetSerializerCount() const
{
	return static_cast<uint4>(m_serializers.size());
}

// Gets all serializers.
template <class Serializable, class Serializer>
void Serialization<Serializable, Serializer>::GetSerializers(const Serializer **serializers) const
{
	for (serializer_map::const_iterator it = m_serializers.begin(); it != m_serializers.end(); ++it)
		*serializers++ = it->second;
}

// Gets an entity serializer for the given entity type, if available, returns nullptr otherwise.
template <class Serializable, class Serializer>
const Serializer* Serialization<Serializable, Serializer>::GetSerializer(const utf8_ntri &type) const
{
	serializer_map::const_iterator itSerializer = m_serializers.find(type.to<utf8_string>());

	return (itSerializer != m_serializers.end())
		? itSerializer->second
		: nullptr;
}

// Loads an entity from the given xml node.
template <class Serializable, class Serializer>
lean::resource_ptr<Serializable, true> Serialization<Serializable, Serializer>::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
	const Serializer *pSerializer = GetSerializer( Serializer::GetType(node) );
	
	return (pSerializer)
		? pSerializer->Load(node, parameters, queue)
		: nullptr;
}

// Saves the given entity to the given XML node.
template <class Serializable, class Serializer>
bool Serialization<Serializable, Serializer>::Save(const Serializable *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const
{
	const Serializer *pSerializer = GetSerializer( pSerializable->GetType() );
	
	if (pSerializer)
		pSerializer->Save(pSerializable, node, parameters, queue);

	return (pSerializer != nullptr);
}

} // namespace
