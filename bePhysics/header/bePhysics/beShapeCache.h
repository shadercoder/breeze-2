/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_SHAPECACHE
#define BE_PHYSICS_SHAPECACHE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>
#include "beShapes.h"
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>
#include <beCore/beDependencies.h>

namespace bePhysics
{

/// Shape cache interface.
class ShapeCache : public lean::noncopyable, public beCore::Resource, public Implementation
{
public:
	virtual ~ShapeCache() throw() { }

	/// Sets the default material.
	virtual void SetDefaultMaterial(const bePhysics::Material *pMaterial) = 0;
	/// Sets the default material.
	virtual const bePhysics::Material* GetDefaultMaterial() const = 0;

	/// Sets the given name for the given shape.
	virtual ShapeCompound* Set(bePhysics::ShapeCompound *shapes, const utf8_ntri &name) = 0;
	/// Sets the given name for the given shape, unsetting the old name.
	virtual void Rename(const bePhysics::ShapeCompound *shapes, const utf8_ntri &name) = 0;
	/// Gets a shape by name.
	virtual ShapeCompound* GetByName(const utf8_ntri &name, bool bThrow = false) const = 0;

	/// Gets a shape by file.
	virtual ShapeCompound* GetByFile(const utf8_ntri &file) = 0;
	/// Sets the given file for the given shape, overriding the old file.
	virtual void Refile(const bePhysics::ShapeCompound *shapes, const utf8_ntri &file) = 0;

	/// Unsets the given shape.
//	virtual void Unset(const bePhysics::ShapeCompound *shapes) = 0;

	/// Gets the main name associated with the given shape.
	virtual utf8_ntr GetName(const bePhysics::ShapeCompound *shapes) const = 0;
	/// Gets the file associated with the given shape.
	virtual utf8_ntr GetFile(const bePhysics::ShapeCompound *shapes) const = 0;

	/// Notifies dependent listeners about dependency changes.
	virtual void NotifyDependents() = 0;
	/// Gets the dependencies registered for the given shape.
	virtual beCore::Dependency<bePhysics::ShapeCompound*>* GetDependency(const bePhysics::ShapeCompound *shapes) = 0;

	/// Gets the path resolver.
	virtual const beCore::PathResolver& GetPathResolver() const = 0;

	/// Gets the device.
	virtual Device* GetDevice() const = 0;
};

/// Creates a physics shape cache.
BE_PHYSICS_API lean::resource_ptr<ShapeCache, true> CreateShapeCache(Device *device, 
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

} // namespace

#endif