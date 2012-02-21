/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidStaticController.h"
#include <beEntitySystem/beEntity.h>
#include "bePhysics/beSceneController.h"
#include "bePhysics/PX/beScene.h"
#include "bePhysics/PX/beRigidActors.h"
#include "bePhysics/PX/beMaterial.h"
#include "bePhysics/PX/beShapes.h"
#include "bePhysics/PX/beMath.h"

namespace bePhysics
{

// Constructor.
RigidStaticController::RigidStaticController(beEntitySystem::Entity *pEntity, SceneController *pScene, RigidStatic *pActor, 
		const ShapeCompound *pShape, const Material *pMaterial)
	: EntityController(pEntity, pScene),
	m_bAttached(false),
	
	m_pActor( LEAN_ASSERT_NOT_NULL(pActor) ),
	m_pShape( pShape ),
	m_pMaterial( pMaterial ),

	m_lastScaling(1.0f)
{
	Synchronize();
}

// Destructor.
RigidStaticController::~RigidStaticController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Forces this controller into synchronization with the simulation.
void RigidStaticController::Synchronize()
{
	if (m_bAttached)
		ToImpl(m_pScene->GetScene())->removeActor(*ToImpl(*m_pActor));

	// NOTE: Not allowed while in scene
	ToImpl(*m_pActor)->setGlobalPose( ToTransform(m_pEntity->GetOrientation(), m_pEntity->GetPosition()) );

	if (m_lastScaling != m_pEntity->GetScaling())
	{
		Scale( *ToImpl(*m_pActor), ToImpl(m_pEntity->GetScaling() / m_lastScaling) );
		m_lastScaling = m_pEntity->GetScaling();
	}

	if (m_bAttached)
		ToImpl(m_pScene->GetScene())->addActor(*ToImpl(*m_pActor));
}

// Sets the material.
void RigidStaticController::SetMaterial(const Material *pMaterial)
{
	if (pMaterial && pMaterial != m_pMaterial)
	{
		const uint4 shapeCount = ToImpl(*m_pActor)->getNbShapes();

		std::vector<physx::PxShape*> shapes(shapeCount);
		ToImpl(*m_pActor)->getShapes(&shapes[0], shapeCount);

		for (size_t i = 0; i < shapeCount; ++i)
			shapes[i]->setMaterials(
					&const_cast<physx::PxMaterial *const &>( ToImpl(*pMaterial).Get() ),
					1
				);

		m_pMaterial = pMaterial;
	}
}

// Attaches this controller to the scene.
void RigidStaticController::Attach()
{
	if (m_bAttached)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_bAttached = true;

	Synchronize();

	ToImpl(m_pScene->GetScene())->addActor(*ToImpl(*m_pActor));
}

// Detaches this controller from the scene.
void RigidStaticController::Detach()
{
	if (!m_bAttached)
		return;

	ToImpl(m_pScene->GetScene())->removeActor(*ToImpl(*m_pActor));

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bAttached = false;
}

// Gets the controller type.
utf8_ntr RigidStaticController::GetControllerType()
{
	return utf8_ntr("RigidStaticController");
}

} // namespace