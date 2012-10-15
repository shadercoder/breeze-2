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
ResourceManager::ResourceManager(class MaterialCache *materialCache, class ShapeCache *shapeCache)
	: m_materialCache( LEAN_ASSERT_NOT_NULL(materialCache) ),
	m_shapeCache( LEAN_ASSERT_NOT_NULL(shapeCache) )
{
}

// Copy constructor.
ResourceManager::ResourceManager(const ResourceManager &right)
	: m_materialCache( right.m_materialCache ),
	m_shapeCache( right.m_shapeCache )
{
}

// Destructor.
ResourceManager::~ResourceManager()
{
}

// Creates a resource manager from the given device.
lean::resource_ptr<ResourceManager, true> CreateResourceManager(Device *device, const utf8_ntri &materialDir, const utf8_ntri &shapeDir)
{
	LEAN_ASSERT(device != nullptr);

	lean::resource_ptr<MaterialCache> effectCache = CreateMaterialCache(device, materialDir);
	lean::resource_ptr<ShapeCache> shapeCache = CreateShapeCache(device, shapeDir);

	return CreateResourceManager(
			effectCache,
			shapeCache
		);
}

// Creates a resource manager from the given effect cache.
lean::resource_ptr<ResourceManager, true> CreateResourceManager(MaterialCache *materialCache, ShapeCache *shapeCache)
{
	return lean::new_resource<ResourceManager>(materialCache, shapeCache);
}

// Notifies dependent listeners about dependency changes.
void NotifyDependents(ResourceManager &resourceManager)
{
	resourceManager.MaterialCache()->NotifyDependents();
	resourceManager.ShapeCache()->NotifyDependents();
}

} // namespace
