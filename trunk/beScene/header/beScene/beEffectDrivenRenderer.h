/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_EFFECT_DRIVEN_RENDERER
#define BE_SCENE_EFFECT_DRIVEN_RENDERER

#include "beScene.h"
#include "beRenderer.h"
#include "beEffectBinderCache.h"
#include <lean/smart/resource_ptr.h>

namespace beScene
{
	
// Prototypes
class PerspectiveEffectBinderPool;
class AbstractRenderableEffectDriver;
class AbstractProcessingEffectDriver;
class RenderableMaterialCache;

/// Renderer.
class EffectDrivenRenderer : public Renderer
{
private:
	const lean::resource_ptr< PerspectiveEffectBinderPool > m_pPerspectivePool;

	const lean::resource_ptr< EffectBinderCache<AbstractRenderableEffectDriver> > m_pRenderableDrivers;

	const lean::resource_ptr< EffectBinderCache<AbstractProcessingEffectDriver> > m_pProcessingDrivers;

	const lean::resource_ptr< RenderableMaterialCache > m_pRenderableMaterials;

public:
	/// Constructor.
	BE_SCENE_API EffectDrivenRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool, class PipePool *pPipePool,
		beScene::RenderingPipeline *pPipeline,
		PerspectiveEffectBinderPool *pPerspectivePool,
		EffectBinderCache<AbstractRenderableEffectDriver> *pRenderableDrivers,
		EffectBinderCache<AbstractProcessingEffectDriver> *pProcessingDrivers,
		RenderableMaterialCache *pRenderableMaterials);
	/// Copy constructor.
	BE_SCENE_API EffectDrivenRenderer(const EffectDrivenRenderer &right);
	/// Destructor.
	BE_SCENE_API ~EffectDrivenRenderer();

	/// Invalidates all caches.
	BE_SCENE_API virtual void InvalidateCaches();

	/// Gets the perspective effect binder pool.
	LEAN_INLINE PerspectiveEffectBinderPool* PerspectivePool() { return m_pPerspectivePool; }
	/// Gets the device.
	LEAN_INLINE const PerspectiveEffectBinderPool* PerspectivePool() const { return m_pPerspectivePool; }

	/// Gets the renderable effect driver cache.
	LEAN_INLINE EffectBinderCache<AbstractRenderableEffectDriver>* RenderableDrivers() { return m_pRenderableDrivers; }
	/// Gets the renderable effect driver cache.
	LEAN_INLINE const EffectBinderCache<AbstractRenderableEffectDriver>* RenderableDrivers() const { return m_pRenderableDrivers; }

	/// Gets the processing effect driver cache.
	LEAN_INLINE EffectBinderCache<AbstractProcessingEffectDriver>* ProcessingDrivers() { return m_pProcessingDrivers; }
	/// Gets the processing effect driver cache.
	LEAN_INLINE const EffectBinderCache<AbstractProcessingEffectDriver>* ProcessingDrivers() const { return m_pProcessingDrivers; }

	/// Gets the renderable effect driver cache.
	LEAN_INLINE RenderableMaterialCache* RenderableMaterials() { return m_pRenderableMaterials; }
	/// Gets the renderable effect driver cache.
	LEAN_INLINE const RenderableMaterialCache* RenderableMaterials() const { return m_pRenderableMaterials; }
};

/// Creates a renderer from the given parameters.
BE_SCENE_API lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(beGraphics::Device *pDevice);
/// Creates a renderer from the given parameters.
BE_SCENE_API lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer);
/// Creates a renderer from the given parameters.
BE_SCENE_API lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer,
	PerspectiveEffectBinderPool *pPerspectivePool);
/// Creates a renderer from the given parameters.
BE_SCENE_API lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer,
	PerspectiveEffectBinderPool *pPerspectivePool, 
	EffectBinderCache<AbstractRenderableEffectDriver> *pRenderableDrivers,
	EffectBinderCache<AbstractProcessingEffectDriver> *pProcessingDrivers);
/// Creates a renderer from the given parameters.
BE_SCENE_API lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer,
	PerspectiveEffectBinderPool *pPerspectivePool, 
	EffectBinderCache<AbstractRenderableEffectDriver> *pRenderableDrivers,
	EffectBinderCache<AbstractProcessingEffectDriver> *pProcessingDrivers,
	RenderableMaterialCache *pRenderableMaterials);

} // namespace

#endif