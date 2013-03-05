/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#pragma once
#ifndef BE_ENTITYSYSTEM_ENTITYCONTROLLER
#define BE_ENTITYSYSTEM_ENTITYCONTROLLER

#include "beEntitySystem.h"
#include "beController.h"
#include <beCore/beShared.h>

namespace beCore
{
	class PersistentIDs;
}

namespace beEntitySystem
{

class Entity;
struct EntityHandle;

/// Entity controller interface.
class LEAN_INTERFACE EntityController : public Controller
{
	LEAN_INTERFACE_BEHAVIOR(EntityController)

public:
	/// Commits large-scale changes, i.e. such affecting the structure of the scene.
	BE_ENTITYSYSTEM_API virtual void Commit(EntityHandle entity);
	/// Explicitly synchronizes this controller with the given
	/// controlled entity. Calls to synchronize are expected to
	/// always imply subsequent calls to flush. Synchronize calls
	/// to entities automatically trigger subsequent flush calls.
	BE_ENTITYSYSTEM_API virtual void Synchronize(EntityHandle entity);
	/// Synchronizes this controller with the given controlled entity.
	BE_ENTITYSYSTEM_API virtual void Flush(const EntityHandle entity);

	/// Attaches this controller to the given entity.
	virtual void Attach(Entity *entity) = 0;
	/// Detaches this controller from the given entity.
	virtual void Detach(Entity *entity) noexcept = 0;

	/// The controller has been added to the given entity.
	BE_ENTITYSYSTEM_API virtual void Added(Entity *entity);
	/// The controller has been removed from the given entity.
	BE_ENTITYSYSTEM_API virtual void Removed(Entity *entity, bool bPermanently) noexcept;

	/// Clones this entity controller.
	virtual EntityController* Clone() const = 0;
	/// Abandons/releases this entity controller.
	virtual void Abandon() const = 0;
};

/// Entity controller interface.
class LEAN_INTERFACE SingularEntityController : public beCore::ResourceAsRefCounted<EntityController>
{
	LEAN_SHARED_INTERFACE_BEHAVIOR(SingularEntityController)

public:
	/// The controller has been added to the given entity.
	BE_ENTITYSYSTEM_API virtual void Added(Entity *entity);
	/// The controller has been removed from the given entity.
	BE_ENTITYSYSTEM_API virtual void Removed(Entity *entity, bool bPermanently) noexcept;

	/// Releases this entity controller.
	BE_ENTITYSYSTEM_API void Abandon() const;
};

/// Deletes the given entity (smart pointer compatibility).
LEAN_INLINE void release_ptr(const EntityController *entity)
{
	if (entity)
		entity->Abandon();
}

} // namespace

#endif