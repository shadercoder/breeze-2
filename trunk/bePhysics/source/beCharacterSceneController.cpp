/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beCharacterSceneController.h"
#include "bePhysics/beSceneController.h"
#include <beEntitySystem/beSimulation.h>
#include "bePhysics/PX3/beCharacterScene.h"
#include "bePhysics/PX3/beScene.h"

namespace bePhysics
{

// Constructor.
CharacterSceneController::CharacterSceneController(beEntitySystem::Simulation *pSimulation, SceneController *pScene, CharacterScene *pCharacters)
	: SimulationController( pSimulation ),
	m_pScene( LEAN_ASSERT_NOT_NULL(pScene) ),
	m_pCharacterScene( LEAN_ASSERT_NOT_NULL(pCharacters) ),
	m_bActive(false)
{
}

// Constructor.
CharacterSceneController::CharacterSceneController(beEntitySystem::Simulation *pSimulation, SceneController *pScene)
	: SimulationController( pSimulation ),
	m_pScene(pScene),
	m_pCharacterScene(
			lean::bind_resource(
				new PX3::CharacterScene(
					PX3::CreateCharacterScene(&ToImpl(*pScene->GetScene())->getPhysics())
				)
			)
		),
	m_bActive(false)
{
}

CharacterSceneController::~CharacterSceneController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Synchronizes this controller with the simulation.
void CharacterSceneController::Flush()
{
	SynchronizedHost::Flush();
}

// Synchronizes the simulation with this controller.
void CharacterSceneController::Fetch()
{
	m_pCharacterScene->Update();

	SynchronizedHost::Fetch();
}

// Runs the physics simulation.
void CharacterSceneController::Step(float timeStep)
{
	AnimatedHost::Step(timeStep);
}

// Attaches this controller to its simulation(s) / data source(s).
void CharacterSceneController::Attach()
{
	if (m_bActive)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_bActive = true;

	m_pScene->AddSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	if (m_pSimulation.check())
		m_pSimulation->AddAnimated(this);
}

// Detaches this controller from its simulation(s) / data source(s).
void CharacterSceneController::Detach()
{
	if (!m_bActive)
		return;

	m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	if (m_pSimulation.check())
		m_pSimulation->RemoveAnimated(this);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bActive = false;
}

// Gets the physics scene.
CharacterScene* CharacterSceneController::GetScene()
{
	return m_pCharacterScene;
}

// Gets the physics scene.
const CharacterScene* CharacterSceneController::GetScene() const
{
	return m_pCharacterScene;
}

// Gets the controller type.
utf8_ntr CharacterSceneController::GetControllerType()
{
	return utf8_ntr("PhysicsCharacterSceneController");
}

} // namespace