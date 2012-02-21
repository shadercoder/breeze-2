/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_ENTITYSERIALIZER
#define BE_ENTITYSYSTEM_ENTITYSERIALIZER

#include "beEntitySystem.h"
#include "beControllerDrivenSerializer.h"

namespace beEntitySystem
{

class Entity;

/// Entity serializer.
class EntitySerializer : public ControllerDrivenSerializer<Entity>
{
public:
	/// Constructor.
	BE_ENTITYSYSTEM_API EntitySerializer(const utf8_ntri &type);
	/// Destructor.
	BE_ENTITYSYSTEM_API ~EntitySerializer();

	/// Creates a serializable object from the given parameters.
	BE_ENTITYSYSTEM_API lean::resource_ptr<Serializable, true> Create(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const;

	/// Loads an entity from the given xml node.
	BE_ENTITYSYSTEM_API virtual lean::resource_ptr<Entity, true> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
	/// Loads an entity from the given xml node.
	BE_ENTITYSYSTEM_API virtual void Load(Entity *pEntity, const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
	/// Saves the given entity object to the given XML node.
	BE_ENTITYSYSTEM_API virtual void Save(const Entity *pEntity, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const;
};

}

#endif