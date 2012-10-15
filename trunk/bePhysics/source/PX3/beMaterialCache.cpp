/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beMaterialCache.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/beMaterialSerialization.h"

#include <beCore/beResourceIndex.h>
#include <beCore/beFileWatch.h>
#include <beCore/beDependenciesImpl.h>

#include <lean/io/filesystem.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

/// Material cache implementation
struct MaterialCache::M : public beCore::FileObserver
{
	MaterialCache *cache;

	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	lean::resource_ptr<Device> device;

	/// Info
	struct Info
	{
		lean::resource_ptr<Material> material;

		beCore::Dependency<bePhysics::Material*> *pDependency;

		/// Constructor.
		Info(lean::resource_ptr<Material> material)
			: material( material.transfer() ),
			pDependency() { }
	};

	typedef beCore::ResourceIndex<bePhysics::Material, Info> resource_index;
	resource_index index;

	beCore::DependenciesImpl<bePhysics::Material*> dependencies;
	beCore::FileWatch fileWatch;

	/// Constructor.
	M(MaterialCache *cache, Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
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
MaterialCache::MaterialCache(bePhysics::Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(this, ToImpl(device), resolver, contentProvider) )
{
}

// Destructor.
MaterialCache::~MaterialCache()
{
}

// Sets the given name for the given shape.
Material* MaterialCache::Set(bePhysics::Material *materialIface, const utf8_ntri &name)
{
	LEAN_PIMPL_M

	Material *material = ToImpl(LEAN_ASSERT_NOT_NULL(materialIface));
	M::resource_index::iterator it = m.index.Find(material);

	// Check, if already cached
	if (it == m.index.End())
	{
		// Insert new resource
		it = m.index.Insert(
				material, name,
				M::Info(material)
			);

		// Allow for monitoring of dependencies
		it->pDependency = m.dependencies.AddDependency(it->material);
	}
	else
		// Add name to cached resource
		m.index.AddName(it, name);

	return material;
}

// Sets the given name for the given shape, unsetting the old name.
void MaterialCache::Rename(const bePhysics::Material *materialIface, const utf8_ntri &name)
{
	LEAN_PIMPL_M

	const Material *material = ToImpl(LEAN_ASSERT_NOT_NULL(materialIface));
	M::resource_index::iterator it = m.index.Find(material);

	if (it != m.index.End())
		m.index.SetName(it, name);
	else
		LEAN_THROW_ERROR_CTX("Unknown resource, renaming failed", name.c_str());
}

// Gets a shape by name.
Material* MaterialCache::GetByName(const utf8_ntri &name, bool bThrow) const
{
	LEAN_PIMPL_CM

	M::resource_index::const_name_iterator it = m.index.FindByName(name.to<utf8_string>());

	if (it != m.index.EndByName())
		return it->material;
	else if (!bThrow)
		return nullptr;
	else
	{
		LEAN_THROW_ERROR_CTX("No material stored under the given name", name.c_str());
		LEAN_ASSERT(false);
	}
}

// Gets a shape by file.
Material* MaterialCache::GetByFile(const utf8_ntri &unresolvedFile)
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

		lean::resource_ptr<Material> material = &ToImpl(*LoadMaterial(*m.device, path, this));

		M::resource_index::iterator it = m.index.Insert(
				material, name,
				M::Info(material)
			);
		itByFile = m.index.SetFile(it, path);

		// Allow for monitoring of dependencies
		it->pDependency = m.dependencies.AddDependency(it->material);

		// Watch material changes
		m.fileWatch.AddObserver(path, &m);
	}
	
	return itByFile->material;
}

// Sets the given file for the given shape, overriding the old file.
void MaterialCache::Refile(const bePhysics::Material *material, const utf8_ntri &file)
{
	LEAN_PIMPL_M

	M::resource_index::iterator it = m.index.Find(LEAN_ASSERT_NOT_NULL(material));

	if (it != m.index.End())
		m.index.SetFile(it, file);
	else
		LEAN_THROW_ERROR_CTX("Unknown resource, refiling failed", file.c_str());
}

// Unsets the given shape.
// void MaterialCache::Unset(const bePhysics::Material *material);

// Gets the main name associated with the given shape.
utf8_ntr MaterialCache::GetName(const bePhysics::Material *material) const
{
	LEAN_PIMPL_CM

	M::resource_index::const_iterator it = m.index.Find(ToImpl(material));

	return (it != m.index.End())
		? m.index.GetName(it)
		: utf8_ntr("");
}

// Gets the file associated with the given shape.
utf8_ntr MaterialCache::GetFile(const bePhysics::Material *material) const
{
	LEAN_PIMPL_CM

	M::resource_index::const_iterator it = m.index.Find(ToImpl(material));

	return (it != m.index.End())
		? m.index.GetFile(it)
		: utf8_ntr("");
}

// Notifies dependent listeners about dependency changes.
void MaterialCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

// Gets the dependencies registered for the given shape.
beCore::Dependency<bePhysics::Material*>* MaterialCache::GetDependency(const bePhysics::Material *material)
{
	LEAN_PIMPL_M

	M::resource_index::iterator it = m.index.Find(ToImpl(material));

	return (it != m.index.End())
		? it->pDependency
		: nullptr;
}

// Gets the path resolver.
const beCore::PathResolver& MaterialCache::GetPathResolver() const
{
	return m->resolver;
}

// Gets the device.
Device* MaterialCache::GetDevice() const
{
	return m->device;
}

// Method called whenever an observed shape has changed.
void MaterialCache::M::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	M &m = *this;

	M::resource_index::file_iterator itByFile = m.index.FindByFile(file.to<utf8_string>());

	if (itByFile != m.index.EndByFile())
	{
		// MONITOR: Non-atomic asynchronous assignment!
		itByFile->material = ToImpl( LoadMaterial(*m.device, itByFile.key(), m.cache).get() );
	
		// Notify dependent listeners
		if (itByFile->pDependency)
			m.dependencies.DependencyChanged(itByFile->pDependency, itByFile->material);
	}
}

} // namespace

// Creates a new material cache.
lean::resource_ptr<MaterialCache, true> CreateMaterialCache(Device *device,
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return lean::bind_resource<MaterialCache>(
			new PX3::MaterialCache( device, resolver, contentProvider )
		);
}

} // namespace