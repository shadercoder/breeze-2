/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beWorld.h"
#include "beEntitySystem/beEntity.h"

#include "beEntitySystem/beEntitySerialization.h"
#include "beEntitySystem/beSerializationParameters.h"
#include "beEntitySystem/beSerializationJob.h"
#include "beEntitySystem/beSerializationTasks.h"

#include <lean/functional/algorithm.h>

#include <lean/xml/xml_file.h>
#include <lean/xml/utility.h>
#include <lean/xml/numeric.h>

#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Creates an empty world.
World::World(const utf8_ntri &name, const WorldDesc &desc)
	: m_name(name.to<utf8_string>()),
	m_desc(desc)
{
}

// Loads the world from the given file.
World::World(const utf8_ntri &name, const utf8_ntri &file, beCore::ParameterSet &parameters, const WorldDesc &desc)
	: m_name(name.to<utf8_string>()),
	m_desc(desc)
{
	lean::xml_file<lean::utf8_t> xml(file);
	rapidxml::xml_node<lean::utf8_t> *root = xml.document().first_node("world");

	if (root)
		LoadWorld(*root, parameters);
	else
		LEAN_THROW_ERROR_CTX("No world node found", file.c_str());
}

// Loads the world from the given XML node.
World::World(const utf8_ntri &name, const rapidxml::xml_node<lean::utf8_t> &node, beCore::ParameterSet &parameters, const WorldDesc &desc)
	: m_name(name.to<utf8_string>()),
	m_desc(desc)
{
	LoadWorld(node, parameters);
}

// Destructor.
World::~World()
{
}

// Adds the given entity to this world.
void World::AddEntity(Entity *pEntity, bool bKeepPersistentID)
{
	if (!pEntity)
	{
		LEAN_LOG_ERROR_MSG("pEntity may not be nullptr");
		return;
	}

	uint4 entityID = static_cast<uint4>( m_entities.size() );
	m_entities.push_back(pEntity);

	try
	{
		// Either insert or add persistent ID
		if (bKeepPersistentID && pEntity->GetPersistentID() != beCore::PersistentIDs::InvalidID)
		{
			if (!m_persistentIDs.SetReference(pEntity->GetPersistentID(), pEntity, true))
				LEAN_THROW_ERROR_CTX("Persistent entity ID collision", pEntity->GetName().c_str());
		}
		else
			pEntity->SetPersistentID( m_persistentIDs.AddReference(pEntity) );
	}
	catch (...)
	{
		m_entities.pop_back();
		throw;
	}

	// Update entity ID
	pEntity->SetID(entityID);
}

// Removes the given entity from this world.
bool World::RemoveEntity(Entity *pEntity, bool bKeepPersistentID)
{
	bool bRemoved = false;

	// Remove entity
	// NOTE: Ordered more expensive, but keeps changes trackable in textual diffs
	{
		entity_vector::iterator itEntity = m_entities.begin();

		// Skip unaffected range
		while (itEntity != m_entities.end() && *itEntity != pEntity)
			++itEntity;

		while (itEntity != m_entities.end())
		{
			// Remove entity from list
			if (*itEntity == pEntity)
			{
				itEntity = m_entities.erase(itEntity);
				bRemoved = true;
			}
			// Update IDs of subsequent entities
			else
			{
				(*itEntity)->SetID( static_cast<uint4>(itEntity - m_entities.begin()) );
				++itEntity;
			}
		}

	}

	if (bRemoved)
	{
		// Invalidate entity ID
		pEntity->SetID(Entity::InvalidID);

		uint8 persistentID = pEntity->GetPersistentID();

		// Invalidate persistent ID
		if (!bKeepPersistentID)
			pEntity->SetPersistentID(beCore::PersistentIDs::InvalidID);

		// Unset persistent reference
		m_persistentIDs.UnsetReference(persistentID, !bKeepPersistentID);
	}

	return bRemoved;
}

// Gets the number of entities.
uint4 World::GetEntityCount() const
{
	return static_cast<uint4>( m_entities.size() );
}

// Gets the n-th entity.
const Entity* World::GetEntity(uint4 id) const
{
	return (id < m_entities.size())
		? m_entities[id]
		: nullptr;
}

// Attaches all entities to their simulations.
void World::Attach() const
{
	for (entity_vector::const_iterator itEntity = m_entities.begin();
		itEntity != m_entities.end(); itEntity++)
		(*itEntity)->Attach();
}

// Detaches all entities from their simulations.
void World::Detach() const
{
	for (entity_vector::const_iterator itEntity = m_entities.begin();
		itEntity != m_entities.end(); itEntity++)
		(*itEntity)->Detach();
}

// Saves the world to the given file.
void World::Serialize(const lean::utf8_ntri &file) const
{
	lean::xml_file<lean::utf8_t> xml;

	rapidxml::xml_node<lean::utf8_t> &root = *lean::allocate_node<utf8_t>(xml.document(), "world");
	// ORDER: Append FIRST, otherwise parent document == nullptr
	xml.document().append_node(&root);
	
	Serialize(root);

	xml.save(file);
}

// Saves the world to the given XML node.
void World::Serialize(rapidxml::xml_node<lean::utf8_t> &node) const
{
	SaveWorld(node);
}

// Saves the world to the given xml node.
void World::SaveWorld(rapidxml::xml_node<utf8_t> &worldNode) const
{
	rapidxml::xml_document<utf8_t> &document = *worldNode.document();

	lean::append_attribute<utf8_t>(document, worldNode, "name", m_name);
	
	// NOTE: Never re-use persistent IDs again
	lean::append_int_attribute<utf8_t>(document, worldNode, "nextPersistentID", m_persistentIDs.GetNextID());

	beCore::ParameterSet parameters(&GetSerializationParameters());
	
	// Execute generic save tasks first
	GetResourceSaveTasks().Save(worldNode, parameters);
	GetWorldSaveTasks().Save(worldNode, parameters);
	
	SaveQueue saveJobs;

	SaveEntities(&m_entities[0].get(), static_cast<uint4>(m_entities.size()), worldNode, &parameters, &saveJobs);

	// Execute any additionally scheduled save jobs
	saveJobs.Save(worldNode, parameters);
}

// Loads the world from the given xml node.
void World::LoadWorld(const rapidxml::xml_node<lean::utf8_t> &worldNode, beCore::ParameterSet &parameters)
{
	lean::get_attribute<utf8_t>(worldNode, "name", m_name);

	// NOTE: Never re-use persistent IDs again
	m_persistentIDs.SkipIDs( lean::get_int_attribute<utf8_t>(worldNode, "nextPersistentID", m_persistentIDs.GetNextID()) );

	// Execute generic load tasks first
	GetResourceLoadTasks().Load(worldNode, parameters);
	GetWorldLoadTasks().Load(worldNode, parameters);

	LoadQueue loadJobs;

	struct WorldInserter : public EntityInserter
	{
		World *world;

		WorldInserter(World &world)
			: world(&world) { }
		
		void GrowEntities(uint4 count)
		{
			world->m_entities.reserve( world->m_entities.size() + count );
		}
		
		void AddEntity(Entity *pEntity)
		{
			world->AddEntity(pEntity, true);
		}
	} inserter(*this);

	LoadEntities(inserter, worldNode, parameters, &loadJobs);

	// Execute any additionally scheduled load jobs
	loadJobs.Load(worldNode, parameters);
}

// Sets the name.
void World::SetName(const utf8_ntri &name)
{
	m_name.assign(name.begin(), name.end());
}

} // namespace
