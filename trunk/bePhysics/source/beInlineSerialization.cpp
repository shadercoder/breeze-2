/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "bePhysicsInternal/stdafx.h"

#include <beEntitySystem/beSerializationTasks.h>

namespace bePhysics
{

const bec::LoadJob *CreateMaterialLoader();
const bec::LoadJob *CreateShapeImportLoader();
const bec::LoadJob *CreateShapeLoader();

namespace
{

struct LoadTaskPlugin
{
	LoadTaskPlugin()
	{
		// ORDER: Loaders are interdependent
		bec::LoadJobs &jobs = beEntitySystem::GetResourceLoadTasks();
		jobs.AddSerializationJob( CreateMaterialLoader() );
		jobs.AddSerializationJob( CreateShapeImportLoader() );
		jobs.AddSerializationJob( CreateShapeLoader() );
	}

} LoadTaskPlugin;

} // namespace

} // namespace
