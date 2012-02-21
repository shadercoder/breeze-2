/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

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
	bool m_bActive;

	lean::resource_ptr<Scene> m_pScene;

	float m_timeStep;

public:
	/// Constructor.
	BE_PHYSICS_API SceneController(beEntitySystem::Simulation *pSimulation, Scene *pScene);
	/// Constructor.
	BE_PHYSICS_API SceneController(beEntitySystem::Simulation *pSimulation, Device &device);
	/// Destructor.
	BE_PHYSICS_API ~SceneController();

	/// Synchronizes this controller with the controller-driven simulation.
	BE_PHYSICS_API void Flush();
	/// Synchronizes the controller-driven simulation with this controller.
	BE_PHYSICS_API void Fetch();

	/// Runs the physics simulation.
	BE_PHYSICS_API void Step(float timeStep);

	/// Attaches this controller to its simulation(s) / data source(s).
	BE_PHYSICS_API void Attach();
	/// Detaches this controller from its simulation(s) / data source(s).
	BE_PHYSICS_API void Detach();

	/// Gets the physics scene.
	BE_PHYSICS_API Scene& GetScene();
	/// Gets the physics scene.
	BE_PHYSICS_API const Scene& GetScene() const;

	/// Gets the controller type.
	BE_PHYSICS_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

} // nmaespace

#endif