/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SERIALIZATION
#define BE_ENTITYSYSTEM_SERIALIZATION

#include "beEntitySystem.h"
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>
#include <lean/rapidxml/rapidxml.hpp>
#include <unordered_map>

// Prototypes
namespace beCore
{
	class ParameterSet;
}

namespace beEntitySystem
{

// Prototypes
template <class Serializable>
class Serializer;

class LoadJob;
class SaveJob;
template <class Job>
class SerializationQueue;

/// Serialization manager.
template < class Serializable, class CustomSerializer = Serializer<Serializable> >
class Serialization : public lean::noncopyable
{
public:
	/// Serializable type.
	typedef Serializable Serializable;
	/// Compatible serializer type.
	typedef CustomSerializer Serializer;

private:
	typedef std::unordered_map<utf8_string, const Serializer*> serializer_map;
	serializer_map m_serializers;

public:
	/// Constructor.
	BE_ENTITYSYSTEM_API Serialization();
	/// Destructor.
	BE_ENTITYSYSTEM_API ~Serialization();

	/// Adds the given serializer to this serialization manager.
	BE_ENTITYSYSTEM_API void AddSerializer(const Serializer *pSerializer);
	/// Removes the given serializer from this serialization manager.
	BE_ENTITYSYSTEM_API bool RemoveSerializer(const Serializer *pSerializer);

	/// Gets the number of serializers.
	BE_ENTITYSYSTEM_API uint4 GetSerializerCount() const;
	/// Gets all serializers.
	BE_ENTITYSYSTEM_API void GetSerializers(const Serializer **serializers) const;

	/// Gets a serializer for the given serializable type, if available, returns nullptr otherwise.
	BE_ENTITYSYSTEM_API const Serializer* GetSerializer(const utf8_ntri &type) const;

	/// Loads an entity from the given xml node.
	BE_ENTITYSYSTEM_API lean::resource_ptr<Serializable, true> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
	/// Saves the given serializable object to the given XML node.
	BE_ENTITYSYSTEM_API bool Save(const Serializable *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const;
};

} // namespace

#endif