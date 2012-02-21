/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beCameraController.h"
#include <beEntitySystem/beEntity.h>
#include "beScene/beSceneController.h"
#include "beScene/bePipe.h"
#include "beScene/bePipelineProcessor.h"
#include "beScene/bePerspectiveScheduler.h"

#include <beCore/beReflectionProperties.h>

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

namespace beScene
{

const beCore::ReflectionProperties CameraControllerProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<float>("fov", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&CameraController::SetFOV) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&CameraController::GetFOV) )
	<< beCore::MakeReflectionProperty<float>("aspect", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&CameraController::SetAspect) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&CameraController::GetAspect) )
	<< beCore::MakeReflectionProperty<float>("near", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&CameraController::SetNearPlane) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&CameraController::GetNearPlane) )
	<< beCore::MakeReflectionProperty<float>("far", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&CameraController::SetFarPlane) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&CameraController::GetFarPlane) )
	<< beCore::MakeReflectionProperty<float>("time", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&CameraController::SetTime) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&CameraController::GetTime) )
	<< beCore::MakeReflectionProperty<float>("time step", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&CameraController::SetTimeStep) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&CameraController::GetTimeStep) );

// Constructor.
CameraController::CameraController(beEntitySystem::Entity *pEntity, SceneController *pScene)
	: EntityController(pEntity, pScene),
	m_bAttached(false),

	m_transform(beMath::fmat4::identity),
	m_view(beMath::fmat4::identity),
	m_proj(beMath::fmat4::identity),
	m_fov(1.0f),
	m_aspect(1.0f),
	m_nearPlane(0.2f),
	m_farPlane(10000.0f),

	m_time(0.0f),
	m_timeStep(0.0f),

	m_pScheduler(nullptr)
{
}

// Destructor.
CameraController::~CameraController()
{
	Detach();
}

// Synchronizes this controller with the controlled entity.
void CameraController::Synchronize()
{
	CameraController::Flush();
}

// Synchronizes this controller with the controlled entity.
void CameraController::Flush()
{
	const beMath::fvec3 &pos = m_pEntity->GetPosition();
	const beMath::fmat3 &orientation = m_pEntity->GetOrientation();

	m_transform = beMath::mat_transform(pos, orientation[2], orientation[1], orientation[0]);
	m_view = beMath::mat_view(pos, orientation[2], orientation[1], orientation[0]);
	m_proj = beMath::mat_proj(m_fov, m_aspect, m_nearPlane, m_farPlane);

	if (m_pScheduler)
		m_pScheduler->AddPerspective(
			PerspectiveFromCamera(this),
			m_pPipe,
			m_pProcessor);
}

// Steps this controller.
void CameraController::Step(float timeStep)
{
	m_timeStep = timeStep;
	m_time += m_timeStep;
}

// Attaches this controller to the scenery.
void CameraController::Attach()
{
	if (m_bAttached)
		return;

	m_bAttached = true;

	m_pScene->AddSynchronized(this, beEntitySystem::SynchronizedFlags::Flush);
	m_pScene->AddAnimated(this);
}

// Detaches this controller from the scenery.
void CameraController::Detach()
{
	m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);
	m_pScene->RemoveAnimated(this);

	m_bAttached = false;
}

// Sets the pipe.
void CameraController::SetPipe(Pipe *pPipe)
{
	m_pPipe = pPipe;
}

// Sets the processor.
void CameraController::SetProcessor(PipelineProcessor *pProcessor)
{
	m_pProcessor = pProcessor;
}

// Sets the perspective scheduler.
void CameraController::SetScheduler(PerspectiveScheduler *pScheduler)
{
	m_pScheduler = pScheduler;
}

// Gets the reflection properties.
CameraController::Properties CameraController::GetControllerProperties()
{
	return Properties(CameraControllerProperties.data(), CameraControllerProperties.data_end());
}

// Gets the reflection properties.
CameraController::Properties CameraController::GetReflectionProperties() const
{
	return Properties(CameraControllerProperties.data(), CameraControllerProperties.data_end());
}

// Gets the controller type.
utf8_ntr CameraController::GetControllerType()
{
	return utf8_ntr("CameraController");
}

// Constructs a perspective description from the given camera controller.
PerspectiveDesc PerspectiveFromCamera(const CameraController *pCamera)
{
	return PerspectiveDesc(
		beMath::fvec3(pCamera->GetMatrix()[3]),
		beMath::fvec3(pCamera->GetMatrix()[0]),
		beMath::fvec3(pCamera->GetMatrix()[1]),
		beMath::fvec3(pCamera->GetMatrix()[2]),
		pCamera->GetViewMatrix(),
		pCamera->GetProjMatrix(),
		pCamera->GetNearPlane(),
		pCamera->GetFarPlane(),
		false,
		pCamera->GetTime(),
		pCamera->GetTimeStep());
}

} // namespace
