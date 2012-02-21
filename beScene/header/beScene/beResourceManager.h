/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RESOURCE_MANAGER
#define BE_SCENE_RESOURCE_MANAGER

#include "beScene.h"
#include <beCore/beShared.h>
#include <beGraphics/beEffectCache.h>
#include <beGraphics/beTextureCache.h>
#include "beMaterialCache.h"
#include "beMeshCache.h"
#include <lean/smart/resource_ptr.h>

namespace beScene
{

/// Resource Manager class.
class ResourceManager : public beCore::Resource
{
private:
	const lean::resource_ptr<beGraphics::EffectCache> m_pEffectCache;

	const lean::resource_ptr<beGraphics::TextureCache> m_pTextureCache;

	const lean::resource_ptr<MaterialCache> m_pMaterialCache;

	const lean::resource_ptr<MeshCache> m_pMeshCache;

public:
	/// Constructor.
	BE_SCENE_API ResourceManager(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache, MaterialCache *pMaterialCache, MeshCache *pMeshCache);
	/// Copy constructor.
	BE_SCENE_API ResourceManager(const ResourceManager &right);
	/// Destructor.
	BE_SCENE_API ~ResourceManager();

	/// Gets the effect cache.
	LEAN_INLINE beGraphics::EffectCache* EffectCache() { return m_pEffectCache; }
	/// Gets the effect cache.
	LEAN_INLINE const beGraphics::EffectCache* EffectCache() const { return m_pEffectCache; }

	/// Gets the texture cache.
	LEAN_INLINE beGraphics::TextureCache* TextureCache() { return m_pTextureCache; }
	/// Gets the texture cache.
	LEAN_INLINE const beGraphics::TextureCache* TextureCache() const { return m_pTextureCache; }

	/// Gets the material cache.
	LEAN_INLINE class MaterialCache* MaterialCache() { return m_pMaterialCache; }
	/// Gets the material cache.
	LEAN_INLINE const class MaterialCache* MaterialCache() const { return m_pMaterialCache; }

	/// Gets the mesh cache.
	LEAN_INLINE class MeshCache* MeshCache() { return m_pMeshCache; }
	/// Gets the mesh cache.
	LEAN_INLINE const class MeshCache* MeshCache() const { return m_pMeshCache; }
};

/// Creates a resource manager from the given device.
BE_SCENE_API lean::resource_ptr<ResourceManager, true> CreateResourceManager(beGraphics::Device *pDevice,
	const utf8_ntri &effectCacheDir, const utf8_ntri &effectDir, const utf8_ntri &textureDir, const utf8_ntri &materialDir, const utf8_ntri &meshDir);
/// Creates a resource manager from the given caches.
BE_SCENE_API lean::resource_ptr<ResourceManager, true> CreateResourceManager(beGraphics::EffectCache *pEffectCache,
	beGraphics::TextureCache *pTextureCache, MaterialCache *pMaterialCache,
	MeshCache *pMeshCache);

/// Notifies dependent listeners about dependency changes.
BE_SCENE_API void NotifyDependents(ResourceManager &resourceManager);

} // namespace

#endif