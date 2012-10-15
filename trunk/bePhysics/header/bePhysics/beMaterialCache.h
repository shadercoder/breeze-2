/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_MATERIALCACHE
#define BE_PHYSICS_MATERIALCACHE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>
#include "beMaterial.h"
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>
#include <beCore/beDependencies.h>

namespace bePhysics
{

/// Material cache interface.
class MaterialCache : public lean::noncopyable, public beCore::Resource, public Implementation
{
public:
	virtual ~MaterialCache() throw() { }

	/// Sets the given name for the given material.
	virtual Material* Set(bePhysics::Material *material, const utf8_ntri &name) = 0;
	/// Sets the given name for the given material, unsetting the old name.
	virtual void Rename(const bePhysics::Material *material, const utf8_ntri &name) = 0;
	/// Gets a material by name.
	virtual Material* GetByName(const utf8_ntri &name, bool bThrow = false) const = 0;

	/// Gets a material by file.
	virtual Material* GetByFile(const utf8_ntri &file) = 0;
	/// Sets the given file for the given material, overriding the old file.
	virtual void Refile(const bePhysics::Material *material, const utf8_ntri &file) = 0;

	/// Unsets the given material.
//	virtual void Unset(const bePhysics::Material *material) = 0;

	/// Gets the main name associated with the given material.
	virtual utf8_ntr GetName(const bePhysics::Material *material) const = 0;
	/// Gets the file associated with the given material.
	virtual utf8_ntr GetFile(const bePhysics::Material *material) const = 0;

	/// Notifies dependent listeners about dependency changes.
	virtual void NotifyDependents() = 0;
	/// Gets the dependencies registered for the given material.
	virtual beCore::Dependency<bePhysics::Material*>* GetDependency(const bePhysics::Material *material) = 0;

	/// Gets the path resolver.
	virtual const beCore::PathResolver& GetPathResolver() const = 0;
	
	/// Gets the device.
	virtual Device* GetDevice() const = 0;
};

/// Creates a physics material cache.
BE_PHYSICS_API lean::resource_ptr<MaterialCache, true> CreateMaterialCache(Device *device,
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

} // namespace

#endif