/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntityGroupController.h"
#include "beEntitySystem/beEntities.h"

#include <lean/functional/algorithm.h>

#include <lean/logging/errors.h>

namespace beEntitySystem
{

BE_CORE_PUBLISH_COMPONENT(EntityGroupController)

namespace
{

template <class It>
void SetOwner(It begin, It end, EntityController *owner) noexcept
{
	for (It it = begin; it < end; ++it)
		(*it)->SetOwner(owner, EntityOwnerNotification::WithoutNotification);
}

template <class It>
void UnsetOwnerWithoutNotification(It begin, It end, EntityController *owner) noexcept
{
	for (It it = begin; it < end; ++it)
		if (Entity *child = *it)
			child->UnsetOwner(owner, EntityOwnerNotification::WithoutNotification);
}

void SetOwner(EntityGroupController::entity_vector &entities, EntityController *owner) noexcept
{
	SetOwner(entities.begin(), entities.end(), owner);
}

void UnsetOwnerWithoutNotification(EntityGroupController::entity_vector &entities, EntityController *owner) noexcept
{
	UnsetOwnerWithoutNotification(entities.begin(), entities.end(), owner);
}

} // namespace

// Creates an empty group.
EntityGroupController::EntityGroupController()
	: m_pOwner()
{
}

// Copies and entity group.
EntityGroupController::EntityGroupController(Entity *const *entities, uint4 count)
	: m_pOwner()
{
	AddEntities(entities, count);
}

// Destructor.
EntityGroupController::~EntityGroupController()
{
	// The following holds if the owner has been removed EXPLICITLY
	// ASSERT: m_pOwner == nullptr
	// ASSERT: No more entities owned
}

// Adds the given entities to this group.
void EntityGroupController::AddEntities(Entity *const *entities, uint4 count)
{
	LEAN_ASSERT(entities != nullptr || count == 0);

	size_t destIdx = m_entities.size();
	m_entities.resize(m_entities.size() + count);
	Entity **dest = &m_entities[destIdx];

	for (Entity *const *src = entities, *const *srcEnd = entities + count; src < srcEnd; ++src, ++dest)
		*dest = LEAN_ASSERT_NOT_NULL(*src);

	if (m_pOwner)
		SetOwner(entities, entities + count, this);
}

// Removes the given entities from this group.
bool EntityGroupController::RemoveEntities(Entity *const *entities, uint4 count)
{
	if (m_pOwner)
		UnsetOwnerWithoutNotification(entities, entities + count, this);

	return lean::remove_all(m_entities, entities, entities + count);
}

// Gets an OPTIONAL parent entity for the children of this controller.
Entity* EntityGroupController::GetParent() const
{
	return m_pOwner;
}

// Gets the rules for (child) entities owned by this controller.
uint4 EntityGroupController::GetChildFlags() const
{
	return ChildEntityFlags::OpenGroup | ChildEntityFlags::Accessible;
}

// The given child entity has been added (== owner about to be set to this).
bool EntityGroupController::ChildAdded(Entity *child)
{
	if (m_pOwner)
	{
		m_entities.push_back(LEAN_ASSERT_NOT_NULL(child));
		return true;
	}
	return false;
}

// The given child entity was removed.
bool EntityGroupController::ChildRemoved(Entity *child)
{
	if (m_pOwner)
		lean::remove(m_entities, child);
	return true;
}

// Attaches all entities to their simulations.
void EntityGroupController::Attach(Entity *entity)
{
	LEAN_ASSERT(!m_pOwner);
	SetOwner(m_entities, this);
	m_pOwner = entity;
}

// Detaches all entities from their simulations.
void EntityGroupController::Detach(Entity *entity)
{
	LEAN_ASSERT(entity == m_pOwner);
	UnsetOwnerWithoutNotification(m_entities, this);
	m_pOwner = nullptr;
}

// Clones this entity controller.
EntityGroupController* EntityGroupController::Clone() const
{
	size_t entityCount = m_entities.size();
	lean::scoped_ptr<lean::scoped_ptr<Entity>[] > entityClones( new lean::scoped_ptr<Entity>[entityCount] );

	// Clone entities
	for (size_t i = 0; i < entityCount; ++i)
		entityClones[i] = m_entities[i]->Clone();

	// Clone controller
	lean::scoped_ptr<EntityGroupController> clone( new EntityGroupController(&entityClones[0].get(), entityCount) );
	for (uint4 i = 0; i < entityCount; ++i) entityClones[i].detach();

	return clone.detach();
}

} // namespace
