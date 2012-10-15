/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_SHAPECACHE_PX
#define BE_PHYSICS_SHAPECACHE_PX

#include "bePhysics.h"
#include "../beShapeCache.h"
#include "beShapes.h"
#include <lean/pimpl/pimpl_ptr.h>
#include "beDevice.h"

namespace bePhysics
{

class Material;

namespace PX3
{

/// Shape cache interface.
class ShapeCache : public bePhysics::ShapeCache
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_PHYSICS_PX3_API ShapeCache(bePhysics::Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_PHYSICS_PX3_API ~ShapeCache();

	/// Sets the default material.
	void SetDefaultMaterial(const bePhysics::Material *pMaterial);
	/// Sets the default material.
	const bePhysics::Material* GetDefaultMaterial() const;

	/// Sets the given name for the given shape.
	BE_PHYSICS_PX3_API ShapeCompound* Set(bePhysics::ShapeCompound *shapes, const utf8_ntri &name);
	/// Sets the given name for the given shape, unsetting the old name.
	BE_PHYSICS_PX3_API void Rename(const bePhysics::ShapeCompound *shapes, const utf8_ntri &name);
	/// Gets a shape by name.
	BE_PHYSICS_PX3_API ShapeCompound* GetByName(const utf8_ntri &name, bool bThrow = false) const;

	/// Gets a shape by file.
	BE_PHYSICS_PX3_API ShapeCompound* GetByFile(const utf8_ntri &file);
	/// Sets the given file for the given shape, overriding the old file.
	BE_PHYSICS_PX3_API void Refile(const bePhysics::ShapeCompound *shapes, const utf8_ntri &file);

	/// Unsets the given shape.
//	BE_PHYSICS_PX3_API void Unset(const bePhysics::ShapeCompound *shapes);

	/// Gets the main name associated with the given shape.
	BE_PHYSICS_PX3_API utf8_ntr GetName(const bePhysics::ShapeCompound *shapes) const;
	/// Gets the file associated with the given shape.
	BE_PHYSICS_PX3_API utf8_ntr GetFile(const bePhysics::ShapeCompound *shapes) const;

	/// Notifies dependent listeners about dependency changes.
	BE_PHYSICS_PX3_API void NotifyDependents();
	/// Gets the dependencies registered for the given shape.
	BE_PHYSICS_PX3_API beCore::Dependency<bePhysics::ShapeCompound*>* GetDependency(const bePhysics::ShapeCompound *shapes);

	/// Gets the path resolver.
	BE_PHYSICS_PX3_API const beCore::PathResolver& GetPathResolver() const;

	/// Gets the device.
	BE_PHYSICS_PX3_API Device* GetDevice() const;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }
};

template <> struct ToImplementationPX<bePhysics::ShapeCache> { typedef ShapeCache Type; };

} // namespace

} // namespace

#endif