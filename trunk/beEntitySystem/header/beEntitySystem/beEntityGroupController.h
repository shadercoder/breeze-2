/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#pragma once
#ifndef BE_ENTITYSYSTEM_ENTITY_GROUP_CONTROLLER
#define BE_ENTITYSYSTEM_ENTITY_GROUP_CONTROLLER

#include "beEntitySystem.h"
#include "beEntityController.h"

namespace beEntitySystem
{

/// Entity group class.
class EntityGroupController : public lean::noncopyable, public SingularEntityController 
{
public:
	typedef std::vector<Entity*> entity_vector;

private:
	entity_vector m_entities;

	Entity *m_pOwner;

public:
	/// Entity Range.
	typedef lean::range<Entity *const *> EntityRange;
	/// Entity Range.
	typedef lean::range<const Entity *const *> ConstEntityRange;

	/// Creates an empty group.
	BE_ENTITYSYSTEM_API EntityGroupController();
	/// Copies and entity group.
	BE_ENTITYSYSTEM_API EntityGroupController(Entity *const *entities, uint4 count);
	/// Destructor.
	BE_ENTITYSYSTEM_API ~EntityGroupController();

	/// Adds the given entities to this group.
	BE_ENTITYSYSTEM_API void AddEntities(Entity *const *entities, uint4 count);
	/// Removes the given entities from this group.
	BE_ENTITYSYSTEM_API bool RemoveEntities(Entity *const *entities, uint4 count);
	/// Adds the given entity to this group.
	void AddEntity(Entity *entity) { AddEntities(&entity, 1); }
	/// Removes the given entity from this group.
	bool RemoveEntity(Entity *pEntity) { return RemoveEntities(&pEntity, 1); }

	/// Gets a range of all entities.
	LEAN_INLINE EntityRange GetEntities()
	{
		return EntityRange( &m_entities[0], &m_entities[0] + m_entities.size() );
	}
	/// Gets a range of all entities.
	LEAN_INLINE ConstEntityRange GetEntities() const
	{
		return ConstEntityRange( &m_entities[0], &m_entities[0] + m_entities.size() );
	}

	/// Gets an OPTIONAL parent entity for the children of this controller.
	BE_ENTITYSYSTEM_API Entity* GetParent() const LEAN_OVERRIDE;
	/// Gets the rules for (child) entities owned by this controller.
	BE_ENTITYSYSTEM_API uint4 GetChildFlags() const LEAN_OVERRIDE;
	/// The given child entity has been added (== owner about to be set to this).
	BE_ENTITYSYSTEM_API bool ChildAdded(Entity *child) LEAN_OVERRIDE;
	/// The given child entity was removed.
	BE_ENTITYSYSTEM_API bool ChildRemoved(Entity *child) LEAN_OVERRIDE;

	/// Attaches all entities to their simulations.
	BE_ENTITYSYSTEM_API void Attach(Entity *entity) LEAN_OVERRIDE;
	/// Detaches all entities from their simulations.
	BE_ENTITYSYSTEM_API void Detach(Entity *entity) LEAN_OVERRIDE;
	
	/// Clones this entity controller.
	BE_ENTITYSYSTEM_API EntityGroupController* Clone() const LEAN_OVERRIDE;

	/// Gets the controller type.
	BE_ENTITYSYSTEM_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_ENTITYSYSTEM_API const beCore::ComponentType* GetType() const LEAN_OVERRIDE;
};

} // namespace

#endif