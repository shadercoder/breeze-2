/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE_MATERIAL_CACHE
#define BE_SCENE_RENDERABLE_MATERIAL_CACHE

#include "beScene.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include "beMaterialCache.h"
#include "beRenderableMaterial.h"
#include <lean/pimpl/pimpl_ptr.h>
#include <beGraphics/DX11/beGraphics.h>
#include <beGraphics/beEffectCache.h>

namespace beScene
{

class AbstractRenderableEffectDriver;
template <class EffectDriver>
class EffectBinderCache;

/// Renderable material cache.
class RenderableMaterialCache : public lean::noncopyable, public beCore::Resource
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_SCENE_API RenderableMaterialCache(EffectBinderCache<AbstractRenderableEffectDriver> *pDriverCache);
	/// Destructor.
	BE_SCENE_API ~RenderableMaterialCache();

	/// Gets a renderable material wrapping the given material.
	BE_SCENE_API RenderableMaterial* GetMaterial(Material *pMaterial);
	
	/// Notifies dependent listeners about dependency changes.
	BE_SCENE_API void NotifyDependents();
	/// Gets the dependencies registered for the given material.
	BE_SCENE_API beCore::Dependency<RenderableMaterial*>* GetDependencies(const RenderableMaterial *pMaterial);
};

/// Creates a new material cache.
BE_SCENE_API lean::resource_ptr<RenderableMaterialCache, true> CreateRenderableMaterialCache(EffectBinderCache<AbstractRenderableEffectDriver> *pDriverCache);

} // namespace

#endif