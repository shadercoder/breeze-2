/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beResourceManager.h"
#include <beCore/beFileSystem.h>

#include <beCore/beFileSystemPathResolver.h>
#include <beCore/beFileContentProvider.h>

namespace bePhysics
{

namespace
{

/// Creates a material cache.
lean::resource_ptr<MaterialCache, true> CreateMaterialCache(Device *device, const utf8_ntri &materialLocation)
{
	return bePhysics::CreateMaterialCache( device, beCore::FileSystemPathResolver(materialLocation), beCore::FileContentProvider() );
}

/// Creates a mesh cache.
lean::resource_ptr<ShapeCache, true> CreateShapeCache(Device *device, const utf8_ntri &shapeLocation)
{
	return bePhysics::CreateShapeCache( device, beCore::FileSystemPathResolver(shapeLocation), beCore::FileContentProvider() );
}

} // namespace

// Constructor.
ResourceManager::ResourceManager(beCore::ComponentMonitor *monitor, class MaterialCache *materialCache,
								 class ShapeCache *shapeCache, class RigidShapeCache *rigidShapeCache)
	: Monitor( LEAN_ASSERT_NOT_NULL(monitor) ),
	MaterialCache( LEAN_ASSERT_NOT_NULL(materialCache) ),
	ShapeCache( LEAN_ASSERT_NOT_NULL(shapeCache) ),
	RigidShapeCache( LEAN_ASSERT_NOT_NULL(rigidShapeCache) )
{
}

// Copy constructor.
ResourceManager::ResourceManager(const ResourceManager &right)
	// MONITOR: REDUNDANT
	: Monitor( right.Monitor ),
	MaterialCache( right.MaterialCache ),
	ShapeCache( right.ShapeCache ),
	RigidShapeCache( right.RigidShapeCache )
{
}

// Destructor.
ResourceManager::~ResourceManager()
{
}

// Commits / reacts to changes.
void ResourceManager::Commit()
{
	MaterialCache->Commit();
	ShapeCache->Commit();
	RigidShapeCache->Commit();
}

// Creates a resource manager from the given device.
lean::resource_ptr<ResourceManager, true> CreateResourceManager(Device *device, const utf8_ntri &materialDir, const utf8_ntri &shapeDir,
	beCore::ComponentMonitor *pMonitor)
{
	LEAN_ASSERT(device != nullptr);

	lean::resource_ptr<beCore::ComponentMonitor> monitor = pMonitor;
	if (!pMonitor)
		monitor = new_resource bec::ComponentMonitor();

	lean::resource_ptr<MaterialCache> materialCache = CreateMaterialCache(device, materialDir);
	lean::resource_ptr<ShapeCache> shapeCache = CreateShapeCache(device, shapeDir);
	
	// TODO: Move to next function?
	materialCache->SetComponentMonitor(monitor);
	shapeCache->SetComponentMonitor(monitor);

	return CreateResourceManager(device, materialCache, shapeCache, nullptr, monitor);
}

// Creates a resource manager from the given effect cache.
lean::resource_ptr<ResourceManager, true> CreateResourceManager(Device *device, MaterialCache *materialCache, ShapeCache *shapeCache,
																RigidShapeCache *pRigidShapeCache,
																beCore::ComponentMonitor *pMonitor)
{
	lean::resource_ptr<beCore::ComponentMonitor> monitor = (!pMonitor) ? new_resource bec::ComponentMonitor() : lean::secure_resource(pMonitor);
	lean::resource_ptr<RigidShapeCache> rigidShapeCache = (!pRigidShapeCache) ? new_resource RigidShapeCache(device) : lean::secure_resource(pRigidShapeCache);
	
	rigidShapeCache->SetComponentMonitor(monitor);

	return new_resource ResourceManager(monitor, materialCache, shapeCache, rigidShapeCache);
}

} // namespace
