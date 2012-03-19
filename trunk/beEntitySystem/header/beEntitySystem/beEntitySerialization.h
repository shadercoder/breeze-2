/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_ENTITYSERIALIZATION
#define BE_ENTITYSYSTEM_ENTITYSERIALIZATION

#include "beEntitySystem.h"
#include "beSerialization.h"

#include <lean/rapidxml/rapidxml.hpp>

namespace beEntitySystem
{

class Entity;
class EntitySerializer;

/// Entity serialization.
typedef Serialization<Entity, EntitySerializer> EntitySerialization;

/// Gets the entity serialization register.
BE_ENTITYSYSTEM_API EntitySerialization& GetEntitySerialization();

/// Instantiate this to add a serializer of the given type.
template <class EntitySerializer>
struct EntitySerializationPlugin
{
	/// Serializer.
	EntitySerializer Serializer;

	/// Adds the serializer.
	EntitySerializationPlugin()
	{
		GetEntitySerialization().AddSerializer(&Serializer);
	}
	/// Removes the serializer.
	~EntitySerializationPlugin()
	{
		GetEntitySerialization().RemoveSerializer(&Serializer);
	}
};

class SaveQueue;
class LoadQueue;

/// Saves the given number of entities to the given xml node.
BE_ENTITYSYSTEM_API void SaveEntities(const Entity * const* entities, uint4 entityCount, rapidxml::xml_node<utf8_t> &parentNode,
	beCore::ParameterSet *pParameters = nullptr, SaveQueue *pQueue = nullptr);

/// Entity inserter interface.
class EntityInserter
{
protected:
	~EntityInserter() { }

public:
	/// Reserves space for the given number of additional entities.
	virtual void GrowEntities(uint4 count) = 0;
	/// Adds the given entity.
	virtual void AddEntity(Entity *pEntity) = 0;
};

/// Loads all entities from the given xml node.
BE_ENTITYSYSTEM_API void LoadEntities(EntityInserter &collection, const rapidxml::xml_node<lean::utf8_t> &parentNode,
	beCore::ParameterSet &parameters, LoadQueue *pQueue = nullptr);

} // namespace

#endif