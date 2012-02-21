/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERER
#define BE_SCENE_RENDERER

#include "beScene.h"
#include <beCore/beShared.h>
#include <beGraphics/beDevice.h>
#include <beGraphics/beDeviceContext.h>
#include <beGraphics/beTextureTargetPool.h>
#include <lean/smart/resource_ptr.h>
#include <lean/pimpl/pimpl_ptr.h>

namespace beScene
{
	
// Prototypes
class RenderingPipeline;
class PipePool;

/// Renderer.
class Renderer : public beCore::Resource
{
private:
	const lean::resource_ptr<beGraphics::Device> m_pDevice;
	const lean::pimpl_ptr<beGraphics::DeviceContext> m_pImmediateContext;

	const lean::resource_ptr<beGraphics::TextureTargetPool> m_pTargetPool;

	const lean::resource_ptr<PipePool> m_pPipePool;

	const lean::resource_ptr<RenderingPipeline> m_pPipeline;

public:
	/// Constructor.
	BE_SCENE_API Renderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool,
		PipePool *pPipePool, beScene::RenderingPipeline *pPipeline);
	/// Copy constructor.
	BE_SCENE_API Renderer(const Renderer &right);
	/// Destructor.
	BE_SCENE_API virtual ~Renderer();

	/// Invalidates all caches.
	BE_SCENE_API virtual void InvalidateCaches();

	/// Gets the device.
	LEAN_INLINE beGraphics::Device* Device() { return m_pDevice; }
	/// Gets the device.
	LEAN_INLINE const beGraphics::Device* Device() const { return m_pDevice; }

	/// Gets the immediate device context.
	LEAN_INLINE const beGraphics::DeviceContext& ImmediateContext() const { return *m_pImmediateContext; }

	/// Gets the immediate device context.
	LEAN_INLINE beGraphics::TextureTargetPool* TargetPool() { return m_pTargetPool; }
	/// Gets the immediate device context.
	LEAN_INLINE const beGraphics::TextureTargetPool* TargetPool() const { return m_pTargetPool; }

	/// Gets the immediate device context.
	LEAN_INLINE PipePool* PipePool() { return m_pPipePool; }
	/// Gets the immediate device context.
	LEAN_INLINE const class PipePool* PipePool() const { return m_pPipePool; }

	/// Gets the pipeline.
	LEAN_INLINE RenderingPipeline* Pipeline() { return m_pPipeline; }
	/// Gets the pipeline.
	LEAN_INLINE const RenderingPipeline* Pipeline() const { return m_pPipeline; }
};

/// Creates a renderer from the given device.
BE_SCENE_API lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice);
/// Creates a renderer from the given device & pool.
BE_SCENE_API lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool);
/// Creates a renderer from the given device & pipeline.
BE_SCENE_API lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beScene::RenderingPipeline *pPipeline);
/// Creates a renderer from the given device, target pool & pipeline.
BE_SCENE_API lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool,
	beScene::RenderingPipeline *pPipeline);
/// Creates a renderer from the given device, target pool & pipeline.
BE_SCENE_API lean::resource_ptr<Renderer, true> CreateRenderer(beGraphics::Device *pDevice, beGraphics::TextureTargetPool *pTargetPool,
	PipePool *pPipePool, beScene::RenderingPipeline *pPipeline);

} // namespace

#endif