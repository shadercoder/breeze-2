/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#pragma once
#ifndef BE_ENTITYSYSTEM_SERIALIZATION_TASKS
#define BE_ENTITYSYSTEM_SERIALIZATION_TASKS

#include "beEntitySystem.h"
#include <beCore/beSerializationJobs.h>

namespace beEntitySystem
{

/// Gets generic tasks to perform when saving.
BE_ENTITYSYSTEM_API beCore::SaveJobs& GetWorldSaveTasks();
/// Gets generic tasks to perform when loading.
BE_ENTITYSYSTEM_API beCore::LoadJobs& GetWorldLoadTasks();

/// Gets generic tasks to perform when saving.
BE_ENTITYSYSTEM_API beCore::SaveJobs& GetResourceSaveTasks();
/// Gets generic tasks to perform when loading.
BE_ENTITYSYSTEM_API beCore::LoadJobs& GetResourceLoadTasks();

} // namespace

#endif