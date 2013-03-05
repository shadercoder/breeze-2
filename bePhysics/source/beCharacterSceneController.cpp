/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beCharacterSceneController.h"
#include "bePhysics/beSceneController.h"
#include <beEntitySystem/beSimulation.h>
#include "bePhysics/PX3/beCharacterScene.h"
#include "bePhysics/PX3/beScene.h"

#include <lean/logging/errors.h>

namespace bePhysics
{

BE_CORE_PUBLISH_COMPONENT(CharacterSceneController)

// Constructor.
CharacterSceneController::CharacterSceneController(SceneController *pScene, CharacterScene *pCharacters)
	: m_scene( LEAN_ASSERT_NOT_NULL(pScene) ),
	m_characterScene( LEAN_ASSERT_NOT_NULL(pCharacters) ),
	m_pAttachedTo()
{
}

// Constructor.
CharacterSceneController::CharacterSceneController(SceneController *pScene)
	: m_scene( LEAN_ASSERT_NOT_NULL(pScene) ),
	m_characterScene(
			lean::bind_resource(
				new PX3::CharacterScene(
					PX3::CreateCharacterScene(&ToImpl(*pScene->GetScene())->getPhysics())
				)
			)
		),
	m_pAttachedTo()
{
}

CharacterSceneController::~CharacterSceneController()
{
}

// Synchronizes this controller with the simulation.
void CharacterSceneController::Flush()
{
	SynchronizedHost::Flush();
}

// Synchronizes the simulation with this controller.
void CharacterSceneController::Fetch()
{
	m_characterScene->Update();

	SynchronizedHost::Fetch();
}

// Runs the physics simulation.
void CharacterSceneController::Step(float timeStep)
{
	AnimatedHost::Step(timeStep);
}

// Attaches this controller to its simulation(s) / data source(s).
void CharacterSceneController::Attach(beEntitySystem::Simulation *simulation)
{
	if (m_pAttachedTo)
	{
		LEAN_LOG_ERROR_MSG("character scene controller already attached to simulation");
		return;
	}

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_pAttachedTo = LEAN_ASSERT_NOT_NULL(simulation);

	m_scene->AddSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	simulation->AddAnimated(this);
}

// Detaches this controller from its simulation(s) / data source(s).
void CharacterSceneController::Detach(beEntitySystem::Simulation *simulation)
{
	if (LEAN_ASSERT_NOT_NULL(simulation) != m_pAttachedTo)
	{
		LEAN_LOG_ERROR_MSG("scene controller was never attached to simulation");
		return;
	}

	m_scene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	simulation->RemoveAnimated(this);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_pAttachedTo = nullptr;
}

// Gets the physics scene.
CharacterScene* CharacterSceneController::GetScene()
{
	return m_characterScene;
}

// Gets the physics scene.
const CharacterScene* CharacterSceneController::GetScene() const
{
	return m_characterScene;
}

} // namespace
