/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beShapeCache.h"

#include <beCore/beResourceIndex.h>
#include <beCore/beResourceManagerImpl.hpp>
#include <beCore/beFileWatch.h>

#include "bePhysics/beAssembledShape.h"
#include "bePhysics/beDevice.h"
#include "bePhysics/beMaterial.h"

#include <lean/smart/cloneable_obj.h>
#include <lean/containers/simple_queue.h>
#include <deque>

#include <lean/io/filesystem.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace bePhysics
{

/// Shape cache implementation
struct ShapeCache::M : public beCore::FileObserver
{
	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	ShapeCache *cache;
	lean::resource_ptr<Device> device;
	
	struct Info
	{
		lean::resource_ptr<AssembledShape> resource;

		Info(AssembledShape *resource)
			: resource(resource) { }
	};

	typedef bec::ResourceIndex<AssembledShape, Info> resources_t;
	resources_t resourceIndex;

	beCore::FileWatch fileWatch;
	typedef lean::simple_queue< std::deque< std::pair< AssembledShape*, lean::resource_ptr<AssembledShape> > > > replace_queue_t;
	replace_queue_t replaceQueue;
	lean::resource_ptr<beCore::ComponentMonitor> pComponentMonitor;

	lean::resource_ptr<Material> defaultMaterial;

	/// Constructor.
	M(ShapeCache *cache, Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: cache(cache),
		device(device),
		resolver(resolver),
		provider(contentProvider)
	{
		LEAN_ASSERT(device != nullptr);

		// TODO: Customizable default material?
		defaultMaterial = CreateMaterial(*device, 1.0f, 1.0f, 0.0f);
	}

	/// Method called whenever an observed mesh has changed.
	void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision) LEAN_OVERRIDE;
};

// Loads a shape from the given file.
lean::resource_ptr<AssembledShape, true> LoadShape(ShapeCache::M &m, const lean::utf8_ntri &file)
{
	lean::com_ptr<beCore::Content> content = m.provider->GetContent(file);
	return LoadShape( content->Bytes(), content->Size(), m.defaultMaterial, *m.device );
}

// Constructor.
ShapeCache::ShapeCache(Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(this, device, resolver, contentProvider) )
{
}

// Destructor.
ShapeCache::~ShapeCache()
{
}

// Gets a shape from the given file.
AssembledShape* ShapeCache::GetByFile(const lean::utf8_ntri &unresolvedFile)
{
	LEAN_PIMPL();

	// Get absolute path
	beCore::Exchange::utf8_string excPath = m.resolver->Resolve(unresolvedFile, true);
	utf8_string path(excPath.begin(), excPath.end());

	// Try to find cached mesh
	M::resources_t::file_iterator it = m.resourceIndex.FindByFile(path);

	if (it == m.resourceIndex.EndByFile())
	{
		LEAN_LOG("Attempting to load shape \"" << path << "\"");
		lean::resource_ptr<AssembledShape> shape = LoadShape(m, path);
		LEAN_LOG("Shape \"" << unresolvedFile.c_str() << "\" created successfully");

		// Insert mesh into cache
		M::resources_t::iterator rit = m.resourceIndex.Insert(
				shape,
				m.resourceIndex.GetUniqueName( lean::get_stem<utf8_string>(unresolvedFile) ),
				M::Info(shape)
			);
		shape->SetCache(this);
		it = m.resourceIndex.SetFile(rit, path);

		// Watch mesh changes
		m.fileWatch.AddObserver(path, &m);
	}

	return it->resource;
}

/// The file associated with the given resource has changed.
LEAN_INLINE void ResourceFileChanged(ShapeCache::M &m, ShapeCache::M::resources_t::iterator it, const utf8_ntri &newFile, const utf8_ntri &oldFile)
{
	// Watch texture changes
	if (!oldFile.empty())
		m.fileWatch.RemoveObserver(oldFile, &m);
	if (!newFile.empty())
		m.fileWatch.AddObserver(newFile, &m);
}

// Commits / reacts to changes.
void ShapeCache::Commit()
{
	LEAN_PIMPL();
	
	bool bHasChanges = !m.replaceQueue.empty();

	while (!m.replaceQueue.empty())
	{
		M::replace_queue_t::value_type replacePair = m.replaceQueue.pop_front();
		Replace(replacePair.first, replacePair.second);
	}

	// Notify dependents
	if (bHasChanges && m.pComponentMonitor)
		m.pComponentMonitor->Replacement.AddChanged(AssembledShape::GetComponentType());
}

// Method called whenever an observed mesh has changed.
void ShapeCache::M::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	LEAN_STATIC_PIMPL();

	M::resources_t::file_iterator it = m.resourceIndex.FindByFile(file.to<utf8_string>());

	if (it != m.resourceIndex.EndByFile())
	{
		M::Info &info = *it;

		LEAN_LOG("Attempting to reload shape \"" << file.c_str() << "\"");
		lean::resource_ptr<AssembledShape> newResource = LoadShape(m, file);
		LEAN_LOG("Shape \"" << file.c_str() << "\" recreated successfully");

		m.replaceQueue.push_back( std::make_pair(info.resource, newResource) );
	}
}

// Sets the component monitor.
void ShapeCache::SetComponentMonitor(beCore::ComponentMonitor *componentMonitor)
{
	m->pComponentMonitor = componentMonitor;
}

// Gets the component monitor.
beCore::ComponentMonitor* ShapeCache::GetComponentMonitor() const
{
	return m->pComponentMonitor;
}

/// Gets the path resolver.
const beCore::PathResolver& ShapeCache::GetPathResolver() const
{
	return m->resolver;
}

// Gets the device.
Device* ShapeCache::GetDevice() const
{
	return m->device;
}
// Creates a new shape cache.
lean::resource_ptr<ShapeCache, true> CreateShapeCache(Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return new_resource ShapeCache(device, resolver, contentProvider);
}

} // namespace