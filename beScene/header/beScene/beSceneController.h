/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_SCENECONTROLLER
#define BE_SCENE_SCENECONTROLLER

#include "beScene.h"
#include <beEntitySystem/beSimulationController.h>
#include <beEntitySystem/beSynchronizedHost.h>
#include <beEntitySystem/beAnimatedHost.h>
#include <beEntitySystem/beRenderable.h>
#include <lean/smart/resource_ptr.h>

namespace beScene
{

// Prototypes
class RenderContext;
class FlatScenery;
class RenderingPipeline;
class DynamicScenery;

/// Scene controller.
class SceneController : public beEntitySystem::SimulationController,
	public beEntitySystem::SynchronizedHost, public beEntitySystem::AnimatedHost, public beEntitySystem::Renderable
{
private:
	bool m_bActive;

	lean::resource_ptr<RenderingPipeline> m_pRenderingPipeline;

	lean::resource_ptr<FlatScenery> m_pScenery;

	lean::resource_ptr<RenderContext> m_pRenderContext;

public:
	/// Constructor.
	BE_SCENE_API SceneController(beEntitySystem::Simulation *pSimulation, RenderingPipeline *pRenderingPipeline,
		RenderContext *pRenderContext);
	/// Destructor.
	BE_SCENE_API ~SceneController();

	/// Synchronizes this controller with the simulation.
	BE_SCENE_API void Flush();

	/// Renders the scene using the stored context.
	BE_SCENE_API void Render();
	/// Renders the scene using the given context.
	BE_SCENE_API void Render(RenderContext &renderContext);

	/// Sets the render context.
	BE_SCENE_API void SetRenderContext(RenderContext *pRenderContext);
	/// Gets the render context.
	LEAN_INLINE RenderContext* GetRenderContext() const { return m_pRenderContext; }

	/// Attaches this controller to its simulation(s) / data source(s).
	BE_SCENE_API void Attach();
	/// Detaches this controller from its simulation(s) / data source(s).
	BE_SCENE_API void Detach();

	/// Gets the managed scenery.
	BE_SCENE_API DynamicScenery* GetScenery();
	/// Gets the managed scenery.
	BE_SCENE_API const DynamicScenery* GetScenery() const;

	/// Gets the pipeline.
	LEAN_INLINE RenderingPipeline* GetRenderingPipeline() { return m_pRenderingPipeline; }
	/// Gets the pipeliney.
	LEAN_INLINE const RenderingPipeline* GetRenderingPipeline() const { return m_pRenderingPipeline; }

	/// Gets the controller type.
	BE_SCENE_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

} // nmaespace

#endif