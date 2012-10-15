/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_MATERIALCACHE_PX
#define BE_PHYSICS_MATERIALCACHE_PX

#include "bePhysics.h"
#include "../beMaterialCache.h"
#include "beMaterial.h"
#include <lean/pimpl/pimpl_ptr.h>
#include "beDevice.h"

namespace bePhysics
{

namespace PX3
{

/// Material cache interface.
class MaterialCache : public bePhysics::MaterialCache
{
public:
	struct M;
	friend struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_PHYSICS_PX3_API MaterialCache(bePhysics::Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_PHYSICS_PX3_API ~MaterialCache();

	/// Sets the given name for the given material.
	BE_PHYSICS_PX3_API Material* Set(bePhysics::Material *material, const utf8_ntri &name);
	/// Sets the given name for the given material, unsetting the old name.
	BE_PHYSICS_PX3_API void Rename(const bePhysics::Material *material, const utf8_ntri &name);
	/// Gets a material by name.
	BE_PHYSICS_PX3_API Material* GetByName(const utf8_ntri &name, bool bThrow = false) const;

	/// Gets a material by file.
	BE_PHYSICS_PX3_API Material* GetByFile(const utf8_ntri &file);
	/// Sets the given file for the given material, overriding the old file.
	BE_PHYSICS_PX3_API void Refile(const bePhysics::Material *material, const utf8_ntri &file);

	/// Unsets the given material.
//	BE_PHYSICS_PX3_API void Unset(const bePhysics::Material *material);

	/// Gets the main name associated with the given material.
	BE_PHYSICS_PX3_API utf8_ntr GetName(const bePhysics::Material *material) const;
	/// Gets the file associated with the given material.
	BE_PHYSICS_PX3_API utf8_ntr GetFile(const bePhysics::Material *material) const;

	/// Notifies dependent listeners about dependency changes.
	BE_PHYSICS_PX3_API void NotifyDependents();
	/// Gets the dependencies registered for the given material.
	BE_PHYSICS_PX3_API beCore::Dependency<bePhysics::Material*>* GetDependency(const bePhysics::Material *material);

	/// Gets the path resolver.
	BE_PHYSICS_PX3_API const beCore::PathResolver& GetPathResolver() const;

	/// Gets the device.
	BE_PHYSICS_PX3_API Device* GetDevice() const;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }
};

template <> struct ToImplementationPX<bePhysics::MaterialCache> { typedef MaterialCache Type; };

} // namespace

} // namespace

#endif