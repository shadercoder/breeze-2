/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntityController.h"
#include "beEntitySystem/beEntities.h"

namespace beEntitySystem
{

// Commits large-scale changes, i.e. such affecting the structure of the scene.
void EntityController::Commit(EntityHandle entity)
{
}

// Synchronizes this controller with the given controlled entity.
void EntityController::Synchronize(EntityHandle entity)
{
}

// Synchronizes this controller with the given controlled entity.
void EntityController::Flush(const EntityHandle entity)
{
}

// The controller has been added to the given entity.
void EntityController::Added(Entity *entity)
{
}

// The controller has been removed from the given entity.
void EntityController::Removed(Entity *entity, bool bPermanently) noexcept
{
	if (bPermanently)
		Abandon();
}

// The controller has been added to the given entity.
void SingularEntityController::Added(Entity *entity)
{
	lean::resource_ptr<SingularEntityController>(this).unbind();
}

// The controller has been removed from the given entity.
void SingularEntityController::Removed(Entity *entity, bool bPermanently) noexcept
{
	lean::resource_ptr<SingularEntityController>(this, lean::bind_reference);
}

// Releases this entity controller.
void SingularEntityController::Abandon() const
{
	lean::resource_ptr<const SingularEntityController>(this, lean::bind_reference);
}

} // namespace
