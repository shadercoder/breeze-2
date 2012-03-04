/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntityGroup.h"
#include "beEntitySystem/beEntity.h"

#include <lean/functional/algorithm.h>

#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Creates an empty group.
EntityGroup::EntityGroup()
{
}

// Destructor.
EntityGroup::~EntityGroup()
{
}

// Adds the given entity to this group.
void EntityGroup::AddEntity(Entity *pEntity, bool bKeepPersistentID)
{
	if (!pEntity)
	{
		LEAN_LOG_ERROR_MSG("pEntity may not be nullptr");
		return;
	}

	uint4 entityID = static_cast<uint4>( m_entities.size() );
	m_entities.push_back(pEntity);
}

// Removes the given entity from this group.
bool EntityGroup::RemoveEntity(Entity *pEntity, bool bKeepPersistentID)
{
	return lean::remove_ordered(m_entities, pEntity);
}

// Attaches all entities to their simulations.
void EntityGroup::Attach() const
{
	for (entity_vector::const_iterator itEntity = m_entities.begin();
		itEntity != m_entities.end(); itEntity++)
		(*itEntity)->Attach();
}

// Detaches all entities from their simulations.
void EntityGroup::Detach() const
{
	for (entity_vector::const_iterator itEntity = m_entities.begin();
		itEntity != m_entities.end(); itEntity++)
		(*itEntity)->Detach();
}

} // namespace
