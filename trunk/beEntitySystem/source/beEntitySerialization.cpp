/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntitySerialization.h"
#include "beEntitySystem/beEntity.h"
#include "beEntitySystem/beEntitySerializer.h"

#include "beEntitySystem/beSerializationParameters.h"
#include "beEntitySystem/beSerializationJob.h"
#include "beEntitySystem/beSerializationTasks.h"

#include <lean/xml/utility.h>

#include <lean/smart/scoped_ptr.h>

#include <lean/logging/errors.h>

namespace beEntitySystem
{
	// Instantiate entity serialization
	template class Serialization<Entity, EntitySerializer>;
}
// Link entity serialization
#include "beSerialization.cpp"

namespace beEntitySystem
{

// Gets the entity serialization register.
EntitySerialization& GetEntitySerialization()
{
	static EntitySerialization manager;
	return manager;
}

// Saves the given number of entities to the given xml node.
void SaveEntities(const Entity * const* entities, uint4 entityCount, rapidxml::xml_node<utf8_t> &parentNode, 
	beCore::ParameterSet *pParameters, SaveQueue *pQueue)
{
	rapidxml::xml_document<utf8_t> &document = *parentNode.document();

	rapidxml::xml_node<utf8_t> &entitiesNode = *lean::allocate_node<utf8_t>(document, "entities");
	// ORDER: Append FIRST, otherwise parent document == nullptrs
	parentNode.append_node(&entitiesNode);

	lean::scoped_ptr<beCore::ParameterSet> pPrivateParameters;

	if (!pParameters)
		pParameters = pPrivateParameters = new beCore::ParameterSet(&GetSerializationParameters());

	lean::scoped_ptr<SaveQueue> pPrivateSaveJobs;

	if (!pQueue)
		pQueue = pPrivateSaveJobs = new SaveQueue();

	const EntitySerialization &entitySerialization = GetEntitySerialization();

	for (uint4 i = 0; i < entityCount; ++i)
	{
		const Entity *pEntity = entities[i];

		rapidxml::xml_node<utf8_t> &entityNode = *lean::allocate_node<utf8_t>(document, "e");
		// ORDER: Append FIRST, otherwise parent document == nullptr
		entitiesNode.append_node(&entityNode);

		entitySerialization.Save(pEntity, entityNode, *pParameters, *pQueue);
	}

	// Execute any additionally scheduled save jobs
	if (pPrivateSaveJobs)
		pPrivateSaveJobs->Save(parentNode, *pParameters);
}

// Loads all entities from the given xml node.
void LoadEntities(EntityInserter &collection, const rapidxml::xml_node<lean::utf8_t> &parentNode,
	beCore::ParameterSet &parameters, LoadQueue *pQueue)
{
	lean::scoped_ptr<LoadQueue> pPrivateLoadJobs;

	if (!pQueue)
		pQueue = pPrivateLoadJobs = new LoadQueue();

	const EntitySerialization &entitySerialization = GetEntitySerialization();

	for (const rapidxml::xml_node<utf8_t> *pEntitiesNode = parentNode.first_node("entities");
		pEntitiesNode; pEntitiesNode = pEntitiesNode->next_sibling("entities"))
	{
		collection.GrowEntities( lean::node_count(*pEntitiesNode) );

		for (const rapidxml::xml_node<utf8_t> *pEntityNode = pEntitiesNode->first_node();
			pEntityNode; pEntityNode = pEntityNode->next_sibling())
		{
			lean::resource_ptr<Entity> pEntity = entitySerialization.Load(*pEntityNode, parameters, *pQueue);

			if (pEntity)
				collection.AddEntity(pEntity);
			else
				LEAN_LOG_ERROR_CTX("LoadEntities()", EntitySerializer::GetName(*pEntityNode));
		}
	}

	// Execute any additionally scheduled load jobs
	if (pPrivateLoadJobs)
		pPrivateLoadJobs->Load(parentNode, parameters);
}

} // namespace
