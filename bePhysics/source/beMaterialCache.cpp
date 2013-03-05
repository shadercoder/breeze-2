/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beMaterialCache.h"

#include "bePhysics/beDevice.h"

#include <beCore/beResourceIndex.h>
#include <beCore/beResourceManagerImpl.hpp>

#include <lean/smart/cloneable_obj.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace bePhysics
{

/// Material cache implementation
struct MaterialCache::M
{
	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	MaterialCache *cache;
	lean::resource_ptr<Device> device;

	struct Info
	{
		lean::resource_ptr<Material> resource;

		Info(Material *resource)
			: resource(resource) { }
	};

	typedef bec::ResourceIndex<Material, Info> resources_t;
	resources_t resourceIndex;

	lean::resource_ptr<beCore::ComponentMonitor> pComponentMonitor;

	/// Constructor.
	M(MaterialCache *cache, Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: cache(cache),
		device(device),
		resolver(resolver),
		provider(contentProvider)
	{
		LEAN_ASSERT(device != nullptr);
	}

};

// Constructor.
MaterialCache::MaterialCache(Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(this, device, resolver, contentProvider) )
{
}

// Destructor.
MaterialCache::~MaterialCache()
{
}

// Commits / reacts to changes.
void MaterialCache::Commit()
{
	LEAN_PIMPL();
}

// Sets the component monitor.
void MaterialCache::SetComponentMonitor(beCore::ComponentMonitor *componentMonitor)
{
	m->pComponentMonitor = componentMonitor;
}

// Gets the component monitor.
beCore::ComponentMonitor* MaterialCache::GetComponentMonitor() const
{
	return m->pComponentMonitor;
}

/// Gets the path resolver.
const beCore::PathResolver& MaterialCache::GetPathResolver() const
{
	return m->resolver;
}

// Gets the device.
Device* MaterialCache::GetDevice() const
{
	return m->device;
}

// Creates a new material cache.
lean::resource_ptr<MaterialCache, true> CreateMaterialCache(Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return new_resource MaterialCache(device, resolver, contentProvider);
}

} // namespace