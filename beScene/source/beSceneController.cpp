/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beSceneController.h"
#include <beEntitySystem/beSimulation.h>
#include "beScene/beFlatScenery.h"
#include "beScene/beRenderingPipeline.h"
#include "beScene/beRenderContext.h"
#include <lean/logging/errors.h>

namespace beScene
{

// Constructor.
SceneController::SceneController(beEntitySystem::Simulation *pSimulation, RenderingPipeline *pRenderingPipeline,
		RenderContext *pRenderContext)
	: SimulationController( pSimulation ),
	m_bActive( false ),
	m_pRenderingPipeline( LEAN_ASSERT_NOT_NULL(pRenderingPipeline) ),
	m_pRenderContext(pRenderContext),
	m_pScenery( lean::bind_resource(new FlatScenery()) )
{
}

// Destructor.
SceneController::~SceneController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Synchronizes this controller with the simulation.
void SceneController::Flush()
{
	// NOTE: Make sure new renderables get flushed right away
	m_pScenery->Prepare();

	SynchronizedHost::Flush();
}

// Renders the scene using the stored context.
void SceneController::Render()
{
	if (m_pRenderContext)
		Render(*m_pRenderContext);
	else
		LEAN_LOG_ERROR_MSG("Cannot render without a valid render context set.");
}

// Renders the scene using the given context.
void SceneController::Render(RenderContext &renderContext)
{
	m_pScenery->Prepare();
	m_pRenderingPipeline->AddScenery(*m_pScenery);
	
	m_pRenderingPipeline->Prepare();
	m_pRenderingPipeline->Render(renderContext);
	
	m_pRenderingPipeline->Release();
}

// Sets the render context.
void SceneController::SetRenderContext(RenderContext *pRenderContext)
{
	m_pRenderContext = pRenderContext;
}

// Attaches this controller to its simulation(s) / data source(s).
void SceneController::Attach()
{
	if (m_bActive)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_bActive = true;

	if (m_pSimulation.check())
	{
		m_pSimulation->AddSynchronized(this, beEntitySystem::SynchronizedFlags::All);
		m_pSimulation->AddAnimated(this);
		m_pSimulation->AddRenderable(this);
	}
}

// Detaches this controller from its simulation(s) / data source(s).
void SceneController::Detach()
{
	if (!m_bActive)
		return;

	if (m_pSimulation.check())
	{
		m_pSimulation->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);
		m_pSimulation->RemoveAnimated(this);
		m_pSimulation->RemoveRenderable(this);
	}

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bActive = false;
}

// Gets the managed scenery.
DynamicScenery* SceneController::GetScenery()
{
	return m_pScenery;
}

// Gets the managed scenery.
const DynamicScenery* SceneController::GetScenery() const
{
	return m_pScenery;
}

// Gets the controller type.
utf8_ntr SceneController::GetControllerType()
{
	return utf8_ntr("SceneController");
}

} // namespace
