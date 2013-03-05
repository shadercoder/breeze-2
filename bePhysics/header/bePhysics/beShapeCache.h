/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_SHAPECACHE
#define BE_PHYSICS_SHAPECACHE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beCore/beResourceManagerImpl.h>
#include <lean/tags/noncopyable.h>
#include <beCore/beComponentMonitor.h>
#include <lean/smart/resource_ptr.h>
#include "beAssembledShape.h"
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>

namespace bePhysics
{

class Device;

/// Shape cache interface.
class ShapeCache : public lean::noncopyable, public beCore::FiledResourceManagerImpl<AssembledShape, ShapeCache>
{
	friend ResourceManagerImpl;
	friend FiledResourceManagerImpl;
	
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_PHYSICS_API ShapeCache(Device *device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_PHYSICS_API ~ShapeCache();

	/// Gets a shape by file.
	BE_PHYSICS_API AssembledShape* GetByFile(const utf8_ntri &file);

	/// Commits / reacts to changes.
	BE_PHYSICS_API void Commit();

	/// Sets the component monitor.
	BE_PHYSICS_API void SetComponentMonitor(beCore::ComponentMonitor *componentMonitor);
	/// Gets the component monitor.
	BE_PHYSICS_API beCore::ComponentMonitor* GetComponentMonitor() const;

	/// Gets the path resolver.
	BE_PHYSICS_API const beCore::PathResolver& GetPathResolver() const;
	/// Gets the device.
	BE_PHYSICS_API Device* GetDevice() const;
};

/// Creates a physics shape cache.
BE_PHYSICS_API lean::resource_ptr<ShapeCache, true> CreateShapeCache(Device *device, 
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

} // namespace

#endif