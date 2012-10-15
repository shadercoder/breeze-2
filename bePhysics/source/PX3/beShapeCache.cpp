/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beShapeCache.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beDevice.h"

#include <beCore/beResourceIndex.h>
#include <beCore/beFileWatch.h>
#include <beCore/beDependenciesImpl.h>

#include <lean/io/filesystem.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

/// Shape cache implementation
struct ShapeCache::M : public beCore::FileObserver
{
	ShapeCache *cache;

	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	lean::resource_ptr<Device> device;

	/// Info
	struct Info
	{
		lean::resource_ptr<ShapeCompound> shapes;

		beCore::Dependency<bePhysics::ShapeCompound*> *pDependency;

		/// Constructor.
		Info(lean::resource_ptr<ShapeCompound> shapes)
			: shapes( shapes.transfer() ),
			pDependency() { }
	};

	typedef beCore::ResourceIndex<bePhysics::ShapeCompound, Info> resource_index;
	resource_index index;

	beCore::DependenciesImpl<bePhysics::ShapeCompound*> dependencies;
	beCore::FileWatch fileWatch;

	lean::resource_ptr<const Material> pDefaultMaterial;

	/// Constructor.
	M(ShapeCache *cache, Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: cache(cache),
		resolver(resolver),
		provider(contentProvider),
		device( LEAN_ASSERT_NOT_NULL(device) )
	{
	}

	/// Method called whenever an observed shape has changed.
	void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision);
};

// Constructor.
ShapeCache::ShapeCache(bePhysics::Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(this, ToImpl(device), resolver, contentProvider) )
{
}

// Destructor.
ShapeCache::~ShapeCache()
{
}

// Sets the default material.
void ShapeCache::SetDefaultMaterial(const bePhysics::Material *pMaterial)
{
	m->pDefaultMaterial = ToImpl(pMaterial);
}

// Sets the default material.
const bePhysics::Material* ShapeCache::GetDefaultMaterial() const
{
	return m->pDefaultMaterial;
}

// Sets the given name for the given shape.
ShapeCompound* ShapeCache::Set(bePhysics::ShapeCompound *shapesIface, const utf8_ntri &name)
{
	LEAN_PIMPL_M

	ShapeCompound *shapes = ToImpl(LEAN_ASSERT_NOT_NULL(shapesIface));
	M::resource_index::iterator it = m.index.Find(shapes);

	// Check, if already cached
	if (it == m.index.End())
	{
		// Insert new resource
		it = m.index.Insert(
				shapes, name,
				M::Info(shapes)
			);

		// Allow for monitoring of dependencies
		it->pDependency = m.dependencies.AddDependency(it->shapes);
	}
	else
		// Add name to cached resource
		m.index.AddName(it, name);

	return shapes;
}

// Sets the given name for the given shape, unsetting the old name.
void ShapeCache::Rename(const bePhysics::ShapeCompound *shapesIface, const utf8_ntri &name)
{
	LEAN_PIMPL_M

	const ShapeCompound *shapes = ToImpl(LEAN_ASSERT_NOT_NULL(shapesIface));
	M::resource_index::iterator it = m.index.Find(shapes);

	if (it != m.index.End())
		m.index.SetName(it, name);
	else
		LEAN_THROW_ERROR_CTX("Unknown resource, renaming failed", name.c_str());
}

// Gets a shape by name.
ShapeCompound* ShapeCache::GetByName(const utf8_ntri &name, bool bThrow) const
{
	LEAN_PIMPL_CM

	M::resource_index::const_name_iterator it = m.index.FindByName(name.to<utf8_string>());

	if (it != m.index.EndByName())
		return it->shapes;
	else if (!bThrow)
		return nullptr;
	else
	{
		LEAN_THROW_ERROR_CTX("No shape stored under the given name", name.c_str());
		LEAN_ASSERT(false);
	}
}

const Material* GetDefaultMaterial(ShapeCache::M &m)
{
	// TODO: HACK?
	if (!m.pDefaultMaterial)
		m.pDefaultMaterial = ToImpl(bePhysics::CreateMaterial(*m.device, 0.9f, 0.8f, 0.1f).get());

	return m.pDefaultMaterial;
}

// Gets a shape by file.
ShapeCompound* ShapeCache::GetByFile(const utf8_ntri &unresolvedFile)
{
	LEAN_PIMPL_M

	beCore::Exchange::utf8_string excPath = m.resolver->Resolve(unresolvedFile, true);
	utf8_string path(excPath.begin(), excPath.end());

	M::resource_index::const_file_iterator itByFile = m.index.FindByFile(path);
	
	// Check, if already cached
	if (itByFile == m.index.EndByFile())
	{
		// Generate unique name from file name
		utf8_string name = m.index.GetUniqueName( lean::get_stem<utf8_string>(unresolvedFile) );

		lean::resource_ptr<ShapeCompound> shapes = &ToImpl(*LoadShape(path, PX3::GetDefaultMaterial(m), *m.device, this));

		M::resource_index::iterator it = m.index.Insert(
				shapes, name,
				M::Info(shapes)
			);
		itByFile = m.index.SetFile(it, path);

		// Allow for monitoring of dependencies
		it->pDependency = m.dependencies.AddDependency(it->shapes);

		// Watch material changes
		m.fileWatch.AddObserver(path, &m);
	}
	
	return itByFile->shapes;
}

// Sets the given file for the given shape, overriding the old file.
void ShapeCache::Refile(const bePhysics::ShapeCompound *shapes, const utf8_ntri &file)
{
	LEAN_PIMPL_M

	M::resource_index::iterator it = m.index.Find(LEAN_ASSERT_NOT_NULL(shapes));

	if (it != m.index.End())
		m.index.SetFile(it, file);
	else
		LEAN_THROW_ERROR_CTX("Unknown resource, refiling failed", file.c_str());
}

// Unsets the given shape.
// void ShapeCache::Unset(const bePhysics::ShapeCompound *shapes);

// Gets the main name associated with the given shape.
utf8_ntr ShapeCache::GetName(const bePhysics::ShapeCompound *shapes) const
{
	LEAN_PIMPL_CM

	M::resource_index::const_iterator it = m.index.Find(ToImpl(shapes));

	return (it != m.index.End())
		? m.index.GetName(it)
		: utf8_ntr("");
}

// Gets the file associated with the given shape.
utf8_ntr ShapeCache::GetFile(const bePhysics::ShapeCompound *shapes) const
{
	LEAN_PIMPL_CM

	M::resource_index::const_iterator it = m.index.Find(ToImpl(shapes));

	return (it != m.index.End())
		? m.index.GetFile(it)
		: utf8_ntr("");
}

// Notifies dependent listeners about dependency changes.
void ShapeCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

// Gets the dependencies registered for the given shape.
beCore::Dependency<bePhysics::ShapeCompound*>* ShapeCache::GetDependency(const bePhysics::ShapeCompound *shapes)
{
	LEAN_PIMPL_M

	M::resource_index::iterator it = m.index.Find(ToImpl(shapes));

	return (it != m.index.End())
		? it->pDependency
		: nullptr;
}

// Gets the path resolver.
const beCore::PathResolver& ShapeCache::GetPathResolver() const
{
	return m->resolver;
}

// Gets the device.
Device* ShapeCache::GetDevice() const
{
	return m->device;
}

// Method called whenever an observed shape has changed.
void ShapeCache::M::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	M &m = *this;

	M::resource_index::file_iterator itByFile = m.index.FindByFile(file.to<utf8_string>());

	if (itByFile != m.index.EndByFile())
	{
		// MONITOR: Non-atomic asynchronous assignment!
		itByFile->shapes = ToImpl( LoadShape(itByFile.key(), PX3::GetDefaultMaterial(m), *m.device, m.cache).get() );
	
		// Notify dependent listeners
		if (itByFile->pDependency)
			m.dependencies.DependencyChanged(itByFile->pDependency, itByFile->shapes);
	}
}

} // namespace

// Creates a new shape cache.
lean::resource_ptr<ShapeCache, true> CreateShapeCache(Device *device,
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return lean::bind_resource<ShapeCache>(
			new PX3::ShapeCache( device, resolver, contentProvider )
		);
}

} // namespace