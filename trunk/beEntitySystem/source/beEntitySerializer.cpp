/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntitySerializer.h"
#include "beEntitySystem/beEntity.h"

#include "beEntitySystem/beControllerSerialization.h"
#include "beEntitySystem/beControllerSerializer.h"

#include "beEntitySystem/beEntitySerialization.h"

#include "beEntitySystem/beSerializationParameters.h"
#include <beCore/bePropertySerialization.h>

namespace beEntitySystem
{
	// Instantiate entity serializer
	template class Serializer<Entity>;
	template class ControllerDrivenSerializer<Entity>;
}
// Link entity serialization
#include "beSerializer.cpp"
#include "beControllerDrivenSerializer.cpp"

namespace beEntitySystem
{
	
// Constructor.
EntitySerializer::EntitySerializer(const utf8_ntri &type)
	: ControllerDrivenSerializer<Entity>(type)
{
}

// Destructor.
EntitySerializer::~EntitySerializer()
{
}

// Creates a serializable object from the given parameters.
lean::resource_ptr<Entity, true> EntitySerializer::Create(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	return lean::new_resource<Entity>( creationParameters.GetValueDefault<beCore::Exchange::utf8_string>("Name").c_str() );
}

// Loads an entity from the given xml node.
lean::resource_ptr<Entity, true> EntitySerializer::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
	lean::resource_ptr<Entity> pEntity = lean::new_resource<Entity>( GetName(node) );
	Load(pEntity, node, parameters, queue);
	return pEntity.transfer();
}

// Loads an entity from the given xml node.
void EntitySerializer::Load(Entity *pEntity, const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
	// Properties
	LoadProperties(*pEntity, node);

	// Controllers
	SetEntityParameter(parameters, pEntity);
	ControllerDrivenSerializer<Entity>::Load(pEntity, node, parameters, queue);
}

// Saves the given entity object to the given XML node.
void EntitySerializer::Save(const Entity *pEntity, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const
{
	SetName(pEntity->GetName(), node);
	SetID(pEntity->GetPersistentID(), node);

	// Properties
	SaveProperties(*pEntity, node);
	
	// Controllers
	ControllerDrivenSerializer<Entity>::Save(pEntity, node, parameters, queue);
}

namespace
{

// Plugin class.
struct EntitySerialization
{
	EntitySerializer Serializer;

	EntitySerialization()
		: Serializer( Entity::GetEntityType() )
	{
		beEntitySystem::GetEntitySerialization().AddSerializer(&Serializer);
	}
	~EntitySerialization()
	{
		beEntitySystem::GetEntitySerialization().RemoveSerializer(&Serializer);
	}
};

const EntitySerialization EntitySerialization;

} // namespace

} // namespace
