/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PROCESSING_EFFECT_DRIVER_CACHE
#define BE_SCENE_PROCESSING_EFFECT_DRIVER_CACHE

#include "beScene.h"
#include "beEffectBinderCache.h"
#include <lean/smart/resource_ptr.h>
#include <lean/pimpl/pimpl_ptr.h>

namespace beScene
{

// Prototypes
class AbstractProcessingEffectDriver;
class RenderingPipeline;
class PerspectiveEffectBinderPool;

/// Effect binder cache implementation.
class ProcessingEffectDriverCache : public EffectBinderCache<AbstractProcessingEffectDriver>
{
private:
	class Impl;
	lean::pimpl_ptr<Impl> m_impl;

protected:
	/// Rendering Pipeline.
	const lean::resource_ptr<RenderingPipeline> m_pPipeline;

	/// Effect binder pool.
	const lean::resource_ptr<PerspectiveEffectBinderPool> m_pPool;

	/// Creates an effect binder from the given effect.
	BE_SCENE_API virtual lean::resource_ptr<AbstractProcessingEffectDriver, true> CreateEffectBinder(const beGraphics::Technique &technique, uint4 flags) const;

public:
	/// Constructor.
	BE_SCENE_API ProcessingEffectDriverCache(RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool);
	/// Destructor.
	BE_SCENE_API ~ProcessingEffectDriverCache();

	/// Gets an effect binder from the given effect.
	BE_SCENE_API AbstractProcessingEffectDriver* GetEffectBinder(const beGraphics::Technique &technique, uint4 flags = 0);
};

} // namespace

#endif