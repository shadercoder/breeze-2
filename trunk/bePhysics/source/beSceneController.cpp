/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beSceneController.h"
#include <beEntitySystem/beSimulation.h>
#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/PX3/beScene.h"

namespace bePhysics
{

// Constructor.
SceneController::SceneController(beEntitySystem::Simulation * pSimulation, Scene *scene)
	: SimulationController( pSimulation ),
	m_pScene( LEAN_ASSERT_NOT_NULL(scene) ),
	m_bActive(false),
	m_timeStep(0.0f)
{
}

// Constructor.
SceneController::SceneController(beEntitySystem::Simulation * pSimulation, Device *device)
	: SimulationController( pSimulation ),
	m_pScene( CreateScene(device, SceneDesc()) ),
	m_bActive(false),
	m_timeStep(0.0f)
{
}

SceneController::~SceneController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Synchronizes this controller with the simulation.
void SceneController::Flush()
{
	SynchronizedHost::Flush();

	// NOTE: Checked runtime requires postive time step
	if (!m_pSimulation->IsPaused() && m_timeStep > 0.0f)
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
	}

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bActive = false;
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

// Gets the controller type.
utf8_ntr SceneController::GetControllerType()
{
	return utf8_ntr("PhysicsSceneController");
}

} // namespace