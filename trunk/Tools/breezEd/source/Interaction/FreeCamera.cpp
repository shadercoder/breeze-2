#include "stdafx.h"
#include "Interaction/FreeCamera.h"

#include "Utility/InputProvider.h"

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>
#include <beMath/beUtility.h>

namespace
{

/// Moves the camera.
void move(beEntitySystem::Entity &entity, float timeStep, float speed, InputProvider &input)
{
	beMath::fvec3 pos = entity.GetPosition();
	const beMath::fmat3 &orientation = entity.GetOrientation();
	
	// Fast / precise
	if (input.keyPressed(Qt::Key_Control))
		speed *= 0.025f;
	if (input.keyPressed(Qt::Key_Shift))
		speed *= 25.0f;
	
	const float step = speed * timeStep;

	// Move along main camera axes
	if (input.keyPressed(Qt::Key_W))
		pos += orientation[2] * step;
	if (input.keyPressed(Qt::Key_S))
		pos -= orientation[2] * step;
	
	if (input.keyPressed(Qt::Key_A))
		pos -= orientation[0] * step;
	if (input.keyPressed(Qt::Key_D))
		pos += orientation[0] * step;

	if (input.keyPressed(Qt::Key_Space))
		pos += orientation[1] * step;
	if (input.keyPressed(Qt::Key_X))
		pos -= orientation[1] * step;

	entity.SetPosition(pos);
}

/// Rotates the camera.
void rotate(beEntitySystem::Entity &entity, float timeStep, InputProvider &input)
{
	if (input.buttonPressed(Qt::RightButton))
	{
		float rotX = beMath::Constants::pi<float>::value * (float) input.relativeDelta().y();
		float rotY = beMath::Constants::pi<float>::value * (float) input.relativeDelta().x();

		beMath::fmat3 orientation = entity.GetOrientation();

		// Rotate
		orientation = mul( orientation, beMath::mat_rot_y<3>( bem::sign(orientation[1].y + 0.5f) * rotY) );
		orientation = mul( beMath::mat_rot_x<3>(rotX), orientation );

		// Re-orientate & orthogonalize
		orientation[1] = normalize( cross(orientation[2], beMath::vec(orientation[0].x, 0.0f, orientation[0].z)) );
		orientation[0] = normalize( cross(orientation[1], orientation[2]) );
		orientation[2] = normalize( orientation[2] );

		entity.SetOrientation(orientation);

		input.nailMouse();
	}
}

} // namespace

// Constructor.
FreeCamera::FreeCamera(beEntitySystem::Entity *pEntity, QObject *pParent)
	: QObject( pParent ),
	m_pEntity( LEAN_ASSERT_NOT_NULL(pEntity) )
{
}

// Destructor.
FreeCamera::~FreeCamera()
{
}

// Steps the interaction.
void FreeCamera::step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective)
{
	move(*m_pEntity, timeStep, 4.0f, input);
	rotate(*m_pEntity, timeStep, input);
}
