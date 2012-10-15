/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_RESOURCE_MANAGER
#define BE_PHYSICS_RESOURCE_MANAGER

#include "bePhysics.h"
#include <beCore/beShared.h>
#include "beDevice.h"
#include "beMaterialCache.h"
#include "beShapeCache.h"
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Resource Manager class.
class ResourceManager : public beCore::Resource
{
private:
	const lean::resource_ptr<MaterialCache> m_materialCache;

	const lean::resource_ptr<ShapeCache> m_shapeCache;

public:
	/// Constructor.
	BE_PHYSICS_API ResourceManager(MaterialCache *materialCache, ShapeCache *shapeCache);
	/// Copy constructor.
	BE_PHYSICS_API ResourceManager(const ResourceManager &right);
	/// Destructor.
	BE_PHYSICS_API ~ResourceManager();

	/// Gets the material cache.
	LEAN_INLINE class MaterialCache* MaterialCache() { return m_materialCache; }
	/// Gets the material cache.
	LEAN_INLINE const class MaterialCache* MaterialCache() const { return m_materialCache; }

	/// Gets the shape cache.
	LEAN_INLINE class ShapeCache* ShapeCache() { return m_shapeCache; }
	/// Gets the shape cache.
	LEAN_INLINE const class ShapeCache* ShapeCache() const { return m_shapeCache; }
};

/// Creates a resource manager from the given device.
BE_PHYSICS_API lean::resource_ptr<ResourceManager, true> CreateResourceManager(Device *device,
	const utf8_ntri &materialDir, const utf8_ntri &shapeDir);
/// Creates a resource manager from the given caches.
BE_PHYSICS_API lean::resource_ptr<ResourceManager, true> CreateResourceManager(MaterialCache *materialCache, ShapeCache *shapeCache);

/// Notifies dependent listeners about dependency changes.
BE_PHYSICS_API void NotifyDependents(ResourceManager &resourceManager);

} // namespace

#endif