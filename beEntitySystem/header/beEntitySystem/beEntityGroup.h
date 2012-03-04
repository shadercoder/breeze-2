/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_ENTITY_GROUP
#define BE_ENTITYSYSTEM_ENTITY_GROUP

#include "beEntitySystem.h"
#include <lean/tags/noncopyable.h>
#include <beCore/beShared.h>
#include <lean/smart/resource_ptr.h>
#include <vector>

namespace beEntitySystem
{

// Prototypes
class Entity;

/// Entity group class.
class EntityGroup : public lean::noncopyable_chain<beCore::Resource>
{
private:
	typedef std::vector< lean::resource_ptr<Entity> > entity_vector;
	entity_vector m_entities;

public:
	/// Creates an empty group.
	BE_ENTITYSYSTEM_API EntityGroup();
	/// Destructor.
	BE_ENTITYSYSTEM_API virtual ~EntityGroup();

	/// Adds the given entity to this group.
	BE_ENTITYSYSTEM_API void AddEntity(Entity *pEntity, bool bKeepPersistentId = false);
	/// Removes the given entity from this group.
	BE_ENTITYSYSTEM_API bool RemoveEntity(Entity *pEntity, bool bKeepPersistentId = false);

	/// Gets the number of entities.
	BE_ENTITYSYSTEM_API uint4 GetEntityCount() const;
	/// Gets the n-th entity.
	LEAN_INLINE Entity* GetEntity(uint4 id)
	{
		return const_cast<Entity*>( const_cast<const EntityGroup*>(this)->GetEntity(id) );
	}
	/// Gets the n-th entity.
	BE_ENTITYSYSTEM_API const Entity* GetEntity(uint4 id) const;

	/// Attaches all entities to their simulations.
	BE_ENTITYSYSTEM_API void Attach() const;
	/// Detaches all entities from their simulations.
	BE_ENTITYSYSTEM_API void Detach() const;
};

} // namespace

#endif