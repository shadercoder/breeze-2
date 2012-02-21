/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_CAMERA_CONTROLLER
#define BE_SCENE_CAMERA_CONTROLLER

#include "beScene.h"
#include "beScene/beEntityController.h"
#include <beEntitySystem/beSynchronized.h>
#include <beEntitySystem/beAnimated.h>
#include <beMath/beMatrixDef.h>
#include <lean/smart/resource_ptr.h>
#include <beScene/bePerspective.h>

namespace beScene
{

// Prototypes
class SceneController;
class Pipe;
class PipelineProcessor;
class PerspectiveScheduler;

/// Camera controller.
class CameraController : public EntityController,
	public beEntitySystem::Synchronized, public beEntitySystem::Animated
{
private:
	bool m_bAttached;

	beMath::fmat4 m_transform;
	beMath::fmat4 m_view;
	beMath::fmat4 m_proj;

	float m_fov;
	float m_aspect;
	float m_nearPlane;
	float m_farPlane;

	float m_time;
	float m_timeStep;

	lean::resource_ptr<Pipe> m_pPipe;
	lean::resource_ptr<PipelineProcessor> m_pProcessor;

	PerspectiveScheduler *m_pScheduler;

public:
	/// Constructor.
	BE_SCENE_API CameraController(beEntitySystem::Entity *pEntity, SceneController *pScene);
	/// Destructor.
	BE_SCENE_API ~CameraController();

	/// Sets the field of view.
	LEAN_INLINE void SetFOV(float fov) { m_fov = fov; }
	/// Gets the field of view.
	LEAN_INLINE float GetFOV() const { return m_fov; }

	/// Sets the aspect ratio.
	LEAN_INLINE void SetAspect(float aspect) { m_aspect = aspect; }
	/// Gets the aspect ratio.
	LEAN_INLINE float GetAspect() const { return m_aspect; }

	/// Sets the near plane.
	LEAN_INLINE void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
	/// Gets the near plane.
	LEAN_INLINE float GetNearPlane() const { return m_nearPlane; }

	/// Sets the far plane.
	LEAN_INLINE void SetFarPlane(float farPlane) { m_farPlane = farPlane; }
	/// Gets the far plane.
	LEAN_INLINE float GetFarPlane() const { return m_farPlane; }

	/// Sets the time.
	LEAN_INLINE void SetTime(float time) { m_time = time; }
	/// Gets the time.
	LEAN_INLINE float GetTime() const { return m_time; }

	/// Sets the time step.
	LEAN_INLINE void SetTimeStep(float timeStep) { m_timeStep = timeStep; }
	/// Gets the time step.
	LEAN_INLINE float GetTimeStep() const { return m_timeStep; }

	/// Synchronizes this controller with the controlled entity.
	BE_SCENE_API void Synchronize();
	/// Synchronizes this controller with the controlled entity.
	BE_SCENE_API void Flush();
	/// Steps this controller.
	BE_SCENE_API void Step(float timeStep);

	/// Attaches this controller.
	BE_SCENE_API void Attach();
	/// Detaches this controller.
	BE_SCENE_API void Detach();

	/// Gets the transformation matrix.
	LEAN_INLINE const beMath::fmat4& GetMatrix() const { return m_transform; } 
	/// Gets the view matrix.
	LEAN_INLINE const beMath::fmat4& GetViewMatrix() const { return m_view; }
	/// Gets the projection matrix.
	LEAN_INLINE const beMath::fmat4& GetProjMatrix() const { return m_proj; }

	/// Sets the pipe.
	BE_SCENE_API void SetPipe(Pipe *pPipe);
	/// Gets the pipe.
	LEAN_INLINE Pipe* GetPipe() const { return m_pPipe; }

	/// Sets the processor.
	BE_SCENE_API void SetProcessor(PipelineProcessor *pProcessor);
	/// Gets the processor.
	LEAN_INLINE PipelineProcessor* GetProcessor() const { return m_pProcessor; }

	/// Sets the perspective scheduler.
	BE_SCENE_API void SetScheduler(PerspectiveScheduler *pScheduler);
	/// Gets the perspective scheduler.
	LEAN_INLINE PerspectiveScheduler* GetScheduler() const { return m_pScheduler; }

	/// Gets the reflection properties.
	BE_SCENE_API static Properties GetControllerProperties();
	/// Gets the reflection properties.
	BE_SCENE_API Properties GetReflectionProperties() const;

	/// Gets the controller type.
	BE_SCENE_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

/// Constructs a perspective description from the given camera controller.
BE_SCENE_API PerspectiveDesc PerspectiveFromCamera(const CameraController *pCamera);

} // namespace

#endif