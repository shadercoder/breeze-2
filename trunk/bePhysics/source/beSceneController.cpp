/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beSceneController.h"
#include <beEntitySystem/beSimulation.h>
#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/PX3/beScene.h"

#include <lean/logging/errors.h>

namespace bePhysics
{

BE_CORE_PUBLISH_COMPONENT_ALIAS(SceneController, "PhysicsSceneController")

// Constructor.
SceneController::SceneController(Scene *scene)
	: m_pScene( LEAN_ASSERT_NOT_NULL(scene) ),
	m_pAttachedTo(),
	m_timeStep(0.0f)
{
}

// Constructor.
SceneController::SceneController(Device *device)
	: m_pScene( CreateScene(device, SceneDesc()) ),
	m_pAttachedTo(),
	m_timeStep(0.0f)
{
}

SceneController::~SceneController()
{
}

// Synchronizes this controller with the simulation.
void SceneController::Flush()
{
	SynchronizedHost::Flush();

	LEAN_ASSERT(m_pAttachedTo);

	// NOTE: Checked runtime requires postive time step
	if (!m_pAttachedTo->IsPaused() && m_timeStep > 0.0f)
	{
		ToImpl(*m_pScene)->simulate(m_timeStep);
		m_timeStep = 0.0f;
	}
}

// Synchronizes the simulation with this controller.
void SceneController::Fetch()
{
	// ORDER: Retrieve simulation results FIRST
	ToImpl(*m_pScene)->fetchResults(true);

	SynchronizedHost::Fetch();
}

// Runs the physics simulation.
void SceneController::Step(float timeStep)
{
	m_timeStep = timeStep;

	AnimatedHost::Step(timeStep);
}

// Attaches this controller to its simulation(s) / data source(s).
void SceneController::Attach(beEntitySystem::Simulation *simulation)
{
	if (m_pAttachedTo)
	{
		LEAN_LOG_ERROR_MSG("physics scene controller already attached to simulation");
		return;
	}

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_pAttachedTo = LEAN_ASSERT_NOT_NULL(simulation);

	simulation->AddSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	simulation->AddAnimated(this);
}

// Detaches this controller from its simulation(s) / data source(s).
void SceneController::Detach(beEntitySystem::Simulation *simulation)
{
	if (LEAN_ASSERT_NOT_NULL(simulation) != m_pAttachedTo)
	{
		LEAN_LOG_ERROR_MSG("physics scene controller controller was never attached to simulation");
		return;
	}

	simulation->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	simulation->RemoveAnimated(this);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_pAttachedTo = nullptr;
}

// Gets the physics scene.
Scene* SceneController::GetScene()
{
	return m_pScene;
}

// Gets the physics scene.
const Scene* SceneController::GetScene() const
{
	return m_pScene;
}

} // namespace