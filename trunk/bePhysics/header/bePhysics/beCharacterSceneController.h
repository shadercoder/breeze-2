/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#pragma once
#ifndef BE_PHYSICS_CHARACTERSCENECONTROLLER
#define BE_PHYSICS_CHARACTERSCENECONTROLLER

#include "bePhysics.h"
#include <beEntitySystem/beSimulationController.h>
#include <beEntitySystem/beSynchronizedHost.h>
#include <beEntitySystem/beAnimatedHost.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

class Device;
class CharacterScene;
class SceneController;

/// Character scene controller.
class CharacterSceneController : public beEntitySystem::SimulationController,
	public beEntitySystem::SynchronizedHost, public beEntitySystem::AnimatedHost
{
private:
	SceneController *m_scene;

	lean::resource_ptr<CharacterScene> m_characterScene;

	beEntitySystem::Simulation *m_pAttachedTo;

public:
	/// Constructor.
	BE_PHYSICS_API CharacterSceneController(SceneController *pScene, CharacterScene *pCharacters);
	/// Constructor.
	BE_PHYSICS_API CharacterSceneController(SceneController *pScene);
	/// Destructor.
	BE_PHYSICS_API ~CharacterSceneController();

	/// Synchronizes this controller with the controller-driven simulation.
	BE_PHYSICS_API void Flush();
	/// Synchronizes the controller-driven simulation with this controller.
	BE_PHYSICS_API void Fetch();

	/// Runs the physics simulation.
	BE_PHYSICS_API void Step(float timeStep);

	/// Attaches this controller to its simulation(s) / data source(s).
	BE_PHYSICS_API void Attach(beEntitySystem::Simulation *simulation);
	/// Detaches this controller from its simulation(s) / data source(s).
	BE_PHYSICS_API void Detach(beEntitySystem::Simulation *simulation);

	/// Gets the character scene.
	BE_PHYSICS_API CharacterScene* GetScene();
	/// Gets the character scene.
	BE_PHYSICS_API const CharacterScene* GetScene() const;

	/// Gets the controller type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const;
};

} // nmaespace

#endif