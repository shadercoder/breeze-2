/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE_EFFECT_DRIVER_CACHE
#define BE_SCENE_RENDERABLE_EFFECT_DRIVER_CACHE

#include "beScene.h"
#include "beEffectBinderCache.h"
#include <lean/smart/resource_ptr.h>
#include <lean/pimpl/pimpl_ptr.h>

namespace beScene
{

// Prototypes
class AbstractRenderableEffectDriver;
class RenderingPipeline;
class PerspectiveEffectBinderPool;

typedef EffectBinderCache<AbstractRenderableEffectDriver> AbstractRenderableEffectDriverCache;

/// Effect binder cache implementation.
class RenderableEffectDriverCache : public EffectBinderCache<AbstractRenderableEffectDriver>
{
public:
	class M;

private:
	lean::pimpl_ptr<M> m;

protected:
	const lean::resource_ptr<RenderingPipeline> m_pipeline;	///< Rendering pipeline.
	PerspectiveEffectBinderPool *const m_pool;				///< Effect binder pool.

	/// Creates an effect binder from the given effect.
	BE_SCENE_API virtual lean::resource_ptr<AbstractRenderableEffectDriver, true> CreateEffectBinder(const beGraphics::Technique &technique, uint4 flags) const;

public:
	/// Constructor.
	BE_SCENE_API RenderableEffectDriverCache(RenderingPipeline *pipeline, PerspectiveEffectBinderPool *pool);
	/// Destructor.
	BE_SCENE_API ~RenderableEffectDriverCache();

	/// Gets an effect binder from the given effect.
	BE_SCENE_API AbstractRenderableEffectDriver* GetEffectBinder(const beGraphics::Technique &technique, uint4 flags = 0);
};

} // namespace

#endif