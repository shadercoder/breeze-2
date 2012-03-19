/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SERIALIZATION_TASKS
#define BE_ENTITYSYSTEM_SERIALIZATION_TASKS

#include "beEntitySystem.h"
#include "beSerializationJob.h"

namespace beEntitySystem
{

/// Gets generic tasks to perform when saving.
BE_ENTITYSYSTEM_API SaveQueue& GetWorldSaveTasks();
/// Gets generic tasks to perform when loading.
BE_ENTITYSYSTEM_API LoadQueue& GetWorldLoadTasks();

/// Gets generic tasks to perform when saving.
BE_ENTITYSYSTEM_API SaveQueue& GetResourceSaveTasks();
/// Gets generic tasks to perform when loading.
BE_ENTITYSYSTEM_API LoadQueue& GetResourceLoadTasks();

} // namespace

#endif