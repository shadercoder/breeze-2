/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MATERIAL_CACHE_DX11
#define BE_SCENE_MATERIAL_CACHE_DX11

#include "../beScene.h"
#include "../beMaterialCache.h"
#include <lean/pimpl/pimpl_ptr.h>
#include <beGraphics/DX11/beGraphics.h>
#include <beGraphics/beEffectCache.h>

namespace beScene
{
namespace DX11
{

/// Material cache implementation.
class MaterialCache : public beScene::MaterialCache
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_SCENE_API MaterialCache(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache,
		const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_SCENE_API ~MaterialCache();

	/// Sets the given name for the given material.
	BE_SCENE_API Material* SetMaterialName(const lean::utf8_ntri &name, Material *pMaterial);
	/// Gets a material by name.
	BE_SCENE_API beScene::Material* GetMaterialByName(const lean::utf8_ntri &name, bool bThrow = false) const;
	
	/// Gets a material from the given effect & name.
	BE_SCENE_API beScene::Material* GetMaterial(const beGraphics::Effect *pEffect, const lean::utf8_ntri &name);
	/// Gets a material from the given file.
	BE_SCENE_API beScene::Material* GetMaterial(const lean::utf8_ntri &file);

	/// Gets the file (or name) of the given mesh.
	BE_SCENE_API utf8_ntr GetFile(const Material *pMaterial, bool *pIsFile = nullptr) const;

	/// Notifies dependent listeners about dependency changes.
	BE_SCENE_API void NotifyDependents();
	/// Gets the dependencies registered for the given material.
	BE_SCENE_API beCore::Dependency<beScene::Material*>* GetDependencies(const beScene::Material *pMesh);

	/// Gets the effect cache.
	BE_SCENE_API beGraphics::EffectCache* GetEffectCache() const;
	/// Gets the texture cache.
	BE_SCENE_API beGraphics::TextureCache* GetTextureCache() const;

	/// Gets the path resolver.
	BE_SCENE_API const beCore::PathResolver& GetPathResolver() const;

	/// Gets the implementation identifier.
	LEAN_INLINE beGraphics::ImplementationID GetImplementationID() const { return beGraphics::DX11Implementation; }
};

using beGraphics::DX11::ToImpl;

} // namespace
} // namespace

namespace beGraphics
{
	namespace DX11
	{
		template <> struct ToImplementationDX11<beScene::MaterialCache> { typedef beScene::DX11::MaterialCache Type; };
	} // namespace
} // namespace

#endif