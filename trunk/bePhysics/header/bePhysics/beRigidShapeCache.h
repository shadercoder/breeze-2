/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_RIGIDSHAPECACHE
#define BE_PHYSICS_RIGIDSHAPECACHE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beCore/beResourceManagerImpl.h>
#include <lean/tags/noncopyable.h>
#include "beRigidShape.h"
#include <beCore/beComponentMonitor.h>
#include <lean/pimpl/pimpl_ptr.h>

namespace bePhysics
{

class Material;

/// Rigid shape cache.
class RigidShapeCache : public lean::noncopyable, public beCore::ResourceManagerImpl<RigidShape, RigidShapeCache>
{
	friend ResourceManagerImpl;

public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_PHYSICS_API RigidShapeCache(Device *device);
	/// Destructor.
	BE_PHYSICS_API ~RigidShapeCache();

	/// Commits / reacts to changes.
	BE_PHYSICS_API void Commit();

	/// Sets a default material.
	BE_PHYSICS_API void SetDefaultMaterial(Material *defaultMaterial);
	/// Gets a default material.
	BE_PHYSICS_API Material* GetDefaultMaterial();

	/// Sets the component monitor.
	BE_PHYSICS_API void SetComponentMonitor(beCore::ComponentMonitor *componentMonitor);
	/// Gets the component monitor.
	BE_PHYSICS_API beCore::ComponentMonitor* GetComponentMonitor() const;
};

/// Creates a new shape cache.
BE_PHYSICS_API lean::resource_ptr<RigidShapeCache, true> CreateRigidShapeCache(Device *device);

} // namespace

#endif