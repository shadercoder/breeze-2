/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beResourceManager.h"
#include <beCore/beFileSystem.h>

#include <beCore/beFileSystemPathResolver.h>
#include <beCore/beFileContentProvider.h>

namespace beScene
{

namespace
{

/// Creates an effect cache.
lean::resource_ptr<beGraphics::EffectCache, true> CreateEffectCache(beGraphics::Device *pDevice,
	const utf8_ntri &effectCacheLocation, const utf8_ntri &effectLocation)
{
	return beGraphics::CreateEffectCache(*pDevice,
		beCore::FileSystem::Get().GetPrimaryPath(effectCacheLocation, true),
		beCore::FileSystemPathResolver(effectLocation), beCore::FileContentProvider() );
}

/// Creates a texture cache.
lean::resource_ptr<beGraphics::TextureCache, true> CreateTextureCache(beGraphics::Device *pDevice,
	const utf8_ntri &textureLocation)
{
	return beGraphics::CreateTextureCache(*pDevice,
		beCore::FileSystemPathResolver(textureLocation), beCore::FileContentProvider() );
}

/// Creates a material cache.
lean::resource_ptr<MaterialCache, true> CreateMaterialCache(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache,
	const utf8_ntri &materialLocation)
{
	return beScene::CreateMaterialCache(pEffectCache, pTextureCache,
		beCore::FileSystemPathResolver(materialLocation), beCore::FileContentProvider() );
}

/// Creates a mesh cache.
lean::resource_ptr<MeshCache, true> CreateMeshCache(beGraphics::Device *pDevice,
	const utf8_ntri &meshLocation)
{
	return beScene::CreateMeshCache(*pDevice,
		beCore::FileSystemPathResolver(meshLocation), beCore::FileContentProvider() );
}

} // namespace

// Constructor.
ResourceManager::ResourceManager(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache, class MaterialCache *pMaterialCache, class MeshCache *pMeshCache)
	: m_pEffectCache( LEAN_ASSERT_NOT_NULL(pEffectCache) ),
	m_pTextureCache( LEAN_ASSERT_NOT_NULL(pTextureCache) ),
	m_pMaterialCache( LEAN_ASSERT_NOT_NULL(pMaterialCache) ),
	m_pMeshCache( LEAN_ASSERT_NOT_NULL(pMeshCache) )
{
}

// Copy constructor.
ResourceManager::ResourceManager(const ResourceManager &right)
	: m_pEffectCache( right.m_pEffectCache ),
	m_pTextureCache( right.m_pTextureCache ),
	m_pMaterialCache( right.m_pMaterialCache ),
	m_pMeshCache( right.m_pMeshCache )
{
}

// Destructor.
ResourceManager::~ResourceManager()
{
}

// Creates a resource manager from the given device.
lean::resource_ptr<ResourceManager, true> CreateResourceManager(beGraphics::Device *pDevice,
	const utf8_ntri &effectCacheDir, const utf8_ntri &effectDir, const utf8_ntri &textureDir, const utf8_ntri &materialDir, const utf8_ntri &meshDir)
{
	LEAN_ASSERT(pDevice != nullptr);

	lean::resource_ptr<beGraphics::EffectCache> pEffectCache = CreateEffectCache(pDevice, effectCacheDir, effectDir);
	lean::resource_ptr<beGraphics::TextureCache> pTextureCache = CreateTextureCache(pDevice, textureDir);

	return CreateResourceManager(
			pEffectCache,
			pTextureCache,
			CreateMaterialCache(pEffectCache, pTextureCache, materialDir).get(),
			CreateMeshCache(pDevice, meshDir).get()
		);
}

// Creates a resource manager from the given effect cache.
lean::resource_ptr<ResourceManager, true> CreateResourceManager(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache,
	MaterialCache *pMaterialCache, MeshCache *pMeshCache)
{
	return lean::bind_resource( new ResourceManager(pEffectCache, pTextureCache, pMaterialCache, pMeshCache) );
}

// Notifies dependent listeners about dependency changes.
void NotifyDependents(ResourceManager &resourceManager)
{
	resourceManager.EffectCache()->NotifyDependents();
	resourceManager.TextureCache()->NotifyDependents();
	resourceManager.MaterialCache()->NotifyDependents();
	resourceManager.MeshCache()->NotifyDependents();
}

} // namespace
