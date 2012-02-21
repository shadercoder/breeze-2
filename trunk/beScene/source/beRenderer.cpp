/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderer.h"
#include "beScene/bePipePool.h"
#include <beGraphics/Any/beDevice.h>
#include <beGraphics/Any/beDeviceContext.h>
#include <beGraphics/Any/beTextureTargetPool.h>
#include "beScene/beRenderingPipeline.h"

namespace beScene
{

namespace
{

/// Creates an immediate device context wrapper.
beGraphics::DeviceContext* CreateImmediateContext(const beGraphics::Device &device)
{
	lean::com_ptr<ID3D11DeviceContext> pContext;
	ToImpl(device)->GetImmediateContext(pContext.rebind());
	return new beGraphics::Any::DeviceContext(pContext);
}

/// Clones a device context wrapper.
beGraphics::DeviceContext* CloneContext(const beGraphics::DeviceContext &context)
{
	return new beGraphics::Any::DeviceContext( ToImpl(context) );
}

// Creates a target pool from the given device.
lean::resource_ptr<beGraphics::TextureTargetPool, true> CreateTargetPool(const beGraphics::Device &device)
{
	return lean::bind_resource(
		new beGraphics::Any::TextureTargetPool(ToImpl(device)) );
}

// Creates a pipe pool from the given device.
lean::resource_ptr<PipePool, true> CreatePipePool(beGraphics::TextureTargetPool *pTargetPool)
{
	return lean::bind_resource(
		new PipePool(pTargetPool) );
}

// Creates a rendering pipeline.
lean::resource_ptr<RenderingPipeline, true> CreatePipeline()
{
	return lean::bind_resource( new RenderingPipeline("Renderer.Pipeline") );
}

}

// Constructor.
Renderer::Renderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool, class PipePool *pPipePool, beScene::RenderingPipeline *pPipeline)
	: m_pDevice( LEAN_ASSERT_NOT_NULL(pDevice) ),
	m_pImmediateContext( CreateImmediateContext(*pDevice) ),
	m_pTargetPool( LEAN_ASSERT_NOT_NULL(pTargetPool) ),
	m_pPipePool( LEAN_ASSERT_NOT_NULL(pPipePool) ),
	m_pPipeline( LEAN_ASSERT_NOT_NULL(pPipeline) )
{
}

// Copy constructor.
Renderer::Renderer(const Renderer &right)
	: m_pDevice( right.m_pDevice ),
	m_pImmediateContext( CloneContext(*right.m_pImmediateContext)  ),
	m_pTargetPool( right.m_pTargetPool ),
	m_pPipePool( right.m_pPipePool ),
	m_pPipeline( right.m_pPipeline )
{
}

// Destructor.
Renderer::~Renderer()
{
}

// Invalidates all caches.
void Renderer::InvalidateCaches()
{
}

// Creates a renderer from the given device.
lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice)
{
	LEAN_ASSERT(pDevice != nullptr);

	return CreateRenderer(pDevice, CreateTargetPool(*pDevice).get(), CreatePipeline().get());
}

// Creates a renderer from the given device & pool.
lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool)
{
	return CreateRenderer(pDevice, pTargetPool, CreatePipeline().get());
}

// Creates a renderer from the given device & pipeline.
lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beScene::RenderingPipeline *pPipeline)
{
	LEAN_ASSERT(pDevice != nullptr);

	return CreateRenderer(pDevice, CreateTargetPool(*pDevice).get(), pPipeline);
}

// Creates a renderer from the given device, target pool & pipeline.
lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool,
	beScene::RenderingPipeline *pPipeline)
{
	LEAN_ASSERT(pTargetPool != nullptr);

	return CreateRenderer(pDevice, pTargetPool, CreatePipePool(pTargetPool).get(), pPipeline);
}

// Creates a renderer from the given device, target pool & pipeline.
lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool,
	PipePool *pPipePool, beScene::RenderingPipeline *pPipeline)
{
	return lean::bind_resource( new Renderer(pDevice, pTargetPool, pPipePool, pPipeline) );
}

} // namespace
