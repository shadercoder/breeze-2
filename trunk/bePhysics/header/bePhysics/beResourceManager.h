/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_RESOURCE_MANAGER
#define BE_PHYSICS_RESOURCE_MANAGER

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beCore/beComponentMonitor.h>
#include "beDevice.h"
#include "beMaterialCache.h"
#include "beShapeCache.h"
#include "beRigidShapeCache.h"
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Resource Manager class.
class ResourceManager : public beCore::Resource
{
public:
	/// Constructor.
	BE_PHYSICS_API ResourceManager(beCore::ComponentMonitor *monitor, class MaterialCache *materialCache,
		class ShapeCache *shapeCache, class RigidShapeCache *rigidShapeCache);
	/// Copy constructor.
	BE_PHYSICS_API ResourceManager(const ResourceManager &right);
	/// Destructor.
	BE_PHYSICS_API ~ResourceManager();
	
	/// Monitor.
	lean::resource_ptr<beCore::ComponentMonitor> Monitor;

	/// Material cache.
	lean::resource_ptr<MaterialCache> MaterialCache;

	/// Shape cache.
	lean::resource_ptr<ShapeCache> ShapeCache;
	/// Rigid shape cache.
	lean::resource_ptr<RigidShapeCache> RigidShapeCache;
	
	/// Commits / reacts to changes.
	BE_PHYSICS_API void Commit();
};

/// Creates a resource manager from the given device.
BE_PHYSICS_API lean::resource_ptr<ResourceManager, true> CreateResourceManager(Device *device,
	const utf8_ntri &materialDir, const utf8_ntri &shapeDir, beCore::ComponentMonitor *pMonitor = nullptr);
/// Creates a resource manager from the given caches.
BE_PHYSICS_API lean::resource_ptr<ResourceManager, true> CreateResourceManager(Device *device, 
																			   MaterialCache *materialCache, ShapeCache *shapeCache,
																			   RigidShapeCache *pRigidShapeCache,
																			   beCore::ComponentMonitor *pMonitor = nullptr);

} // namespace

#endif