/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MATERIAL_CACHE
#define BE_SCENE_MATERIAL_CACHE

#include "beScene.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include "beMaterial.h"
#include <beGraphics/beEffect.h>
#include <beGraphics/beEffectCache.h>
#include <beGraphics/beTextureCache.h>
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>
#include <beCore/beDependencies.h>

namespace beScene
{

/// Mesh cache base.
class MaterialCache : public lean::noncopyable, public beCore::Resource, public beGraphics::Implementation
{
public:
	virtual ~MaterialCache() throw() { }

	/// Sets the given name for the given material.
	virtual Material* SetMaterialName(const lean::utf8_ntri &name, Material *pMaterial) = 0;
	/// Gets a material by name.
	virtual beScene::Material* GetMaterialByName(const lean::utf8_ntri &name, bool bThrow = false) const = 0;

	/// Gets a material from the given effect & name.
	virtual Material* GetMaterial(const beGraphics::Effect *pEffect, const lean::utf8_ntri &name) = 0;
	/// Gets a material from the given file.
	virtual Material* GetMaterial(const lean::utf8_ntri &file) = 0;

	/// Gets the file (or name) of the given material.
	virtual utf8_ntr GetFile(const Material *pMaterial, bool *pIsFile = nullptr) const = 0;

	/// Notifies dependent listeners about dependency changes.
	virtual void NotifyDependents() = 0;
	/// Gets the dependencies registered for the given material.
	virtual beCore::Dependency<Material*>* GetDependencies(const Material *pMesh) = 0;

	/// Gets the effect cache.
	virtual beGraphics::EffectCache* GetEffectCache() const = 0;
	/// Gets the texture cache.
	virtual beGraphics::TextureCache* GetTextureCache() const = 0;

	/// Gets the path resolver.
	virtual const beCore::PathResolver& GetPathResolver() const = 0;
};

/// Creates a new material cache.
BE_SCENE_API lean::resource_ptr<MaterialCache, true> CreateMaterialCache(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache, 
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

} // namespace

#endif