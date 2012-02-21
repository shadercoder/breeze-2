/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beEffectDrivenRenderer.h"
#include "beScene/bePerspectiveEffectBinderPool.h"
#include "beScene/beRenderableEffectDriverCache.h"
#include "beScene/beProcessingEffectDriverCache.h"
#include "beScene/beRenderableMaterialCache.h"
#include <beGraphics/Any/beDevice.h>

namespace beScene
{

namespace
{

// Creates a perspective effect binder pool.
lean::resource_ptr<PerspectiveEffectBinderPool, true> CreatePerspectivePool(const beGraphics::Device &device)
{
	return lean::bind_resource(
		new PerspectiveEffectBinderPool(ToImpl(device)) );
}

// Creates a renderable effect driver cache.
lean::resource_ptr<RenderableEffectDriverCache, true> CreateRenderableDriverCache(RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool)
{
	return lean::bind_resource(
		new RenderableEffectDriverCache(pPipeline, pPool) );
}

// Creates a renderable effect driver cache.
lean::resource_ptr<ProcessingEffectDriverCache, true> CreateProcessingDriverCache(RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool)
{
	return lean::bind_resource(
		new ProcessingEffectDriverCache(pPipeline, pPool) );
}

// Creates a renderable material cache.
lean::resource_ptr<RenderableMaterialCache, true> CreateRenderableMaterialCache(RenderableEffectDriverCache *pRenderableDrivers)
{
	return lean::bind_resource(
		new RenderableMaterialCache(pRenderableDrivers) );
}

} // namespace

// Constructor.
EffectDrivenRenderer::EffectDrivenRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool, class PipePool *pPipePool,
		beScene::RenderingPipeline *pPipeline,
		PerspectiveEffectBinderPool *pPerspectivePool,
		EffectBinderCache<AbstractRenderableEffectDriver> *pRenderableDrivers,
		EffectBinderCache<AbstractProcessingEffectDriver> *pProcessingDrivers,
		RenderableMaterialCache *pRenderableMaterials)
	: Renderer(pDevice, pTargetPool, pPipePool, pPipeline),
	m_pPerspectivePool( LEAN_ASSERT_NOT_NULL(pPerspectivePool) ),
	m_pRenderableDrivers( LEAN_ASSERT_NOT_NULL(pRenderableDrivers) ),
	m_pProcessingDrivers( LEAN_ASSERT_NOT_NULL(pProcessingDrivers) ),
	m_pRenderableMaterials( LEAN_ASSERT_NOT_NULL(pRenderableMaterials) )
{
}

// Copy constructor.
EffectDrivenRenderer::EffectDrivenRenderer(const EffectDrivenRenderer &right)
	: Renderer(right),
	m_pPerspectivePool( LEAN_ASSERT_NOT_NULL(right.m_pPerspectivePool) ),
	m_pRenderableDrivers( LEAN_ASSERT_NOT_NULL(right.m_pRenderableDrivers) ),
	m_pProcessingDrivers( LEAN_ASSERT_NOT_NULL(right.m_pProcessingDrivers) ),
	m_pRenderableMaterials( LEAN_ASSERT_NOT_NULL(right.m_pRenderableMaterials) )
{
}

// Destructor.
EffectDrivenRenderer::~EffectDrivenRenderer()
{
}

// Invalidates all caches.
void EffectDrivenRenderer::InvalidateCaches()
{
	Renderer::InvalidateCaches();
	m_pPerspectivePool->Invalidate();
}

// Creates a renderer from the given parameters.
lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(beGraphics::Device *pDevice)
{
	return CreateEffectDrivenRenderer( *CreateRenderer(pDevice) );
}

// Creates a renderer from the given parameters.
lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer)
{
	return CreateEffectDrivenRenderer(
		renderer,
		CreatePerspectivePool(*renderer.Device()).get() );
}

// Creates a renderer from the given parameters.
lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer,
	PerspectiveEffectBinderPool *pPerspectivePool)
{
	return CreateEffectDrivenRenderer(
		renderer,
		pPerspectivePool,
		CreateRenderableDriverCache(renderer.Pipeline(), pPerspectivePool).get(),
		CreateProcessingDriverCache(renderer.Pipeline(), pPerspectivePool).get() );
}

// Creates a renderer from the given parameters.
lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer,
	PerspectiveEffectBinderPool *pPerspectivePool, 
	EffectBinderCache<AbstractRenderableEffectDriver> *pRenderableDrivers,
	EffectBinderCache<AbstractProcessingEffectDriver> *pProcessingDrivers)
{
	return CreateEffectDrivenRenderer(
		renderer,
		pPerspectivePool,
		pRenderableDrivers, pProcessingDrivers,
		CreateRenderableMaterialCache(pRenderableDrivers).get() );
}

// Creates a renderer from the given parameters.
lean::resource_ptr<EffectDrivenRenderer, true> CreateEffectDrivenRenderer(Renderer &renderer,
	PerspectiveEffectBinderPool *pPerspectivePool, 
	EffectBinderCache<AbstractRenderableEffectDriver> *pRenderableDrivers,
	EffectBinderCache<AbstractProcessingEffectDriver> *pProcessingDrivers,
	RenderableMaterialCache *pRenderableMaterials)
{
	return lean::new_resource<EffectDrivenRenderer>(
		renderer.Device(), renderer.TargetPool(), renderer.PipePool(), renderer.Pipeline(),
		pPerspectivePool,
		pRenderableDrivers, pProcessingDrivers,
		pRenderableMaterials );
}

} // namespace
