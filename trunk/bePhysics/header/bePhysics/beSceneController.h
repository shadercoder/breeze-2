/******************************************************/
/* breeze Engine Physics Module  (c) Tobias Zirr 2011 */
/******************************************************/

#pragma once
#ifndef BE_PHYSICS_SCENECONTROLLER
#define BE_PHYSICS_SCENECONTROLLER

#include "bePhysics.h"
#include <beEntitySystem/beSimulationController.h>
#include <beEntitySystem/beSynchronizedHost.h>
#include <beEntitySystem/beAnimatedHost.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

class Device;
class Scene;

/// Scene controller.
class SceneController : public beEntitySystem::SimulationController,
	public beEntitySystem::SynchronizedHost, public beEntitySystem::AnimatedHost
{
private:
	lean::resource_ptr<Scene> m_pScene;

	beEntitySystem::Simulation *m_pAttachedTo;

	float m_timeStep;

public:
	/// Constructor.
	BE_PHYSICS_API SceneController(Scene *scene);
	/// Constructor.
	BE_PHYSICS_API SceneController(Device *device);
	/// Destructor.
	BE_PHYSICS_API ~SceneController();

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

	/// Gets the physics scene.
	BE_PHYSICS_API Scene* GetScene();
	/// Gets the physics scene.
	BE_PHYSICS_API const Scene* GetScene() const;

	/// Gets the controller type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const;
};

} // nmaespace

#endif