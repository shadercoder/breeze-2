/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidStaticController.h"
#include <beEntitySystem/beEntity.h>
#include "bePhysics/beSceneController.h"
#include "bePhysics/PX3/beScene.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beMath.h"

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
		ToImpl(*m_pScene->GetScene())->removeActor(*ToImpl(*m_pActor));

	// NOTE: Not allowed while in scene
	ToImpl(*m_pActor)->setGlobalPose( PX3::ToTransform(m_pEntity->GetOrientation(), m_pEntity->GetPosition()) );

	if (m_lastScaling != m_pEntity->GetScaling())
	{
		PX3::Scale( *ToImpl(*m_pActor), PX3::ToAPI(m_pEntity->GetScaling() / m_lastScaling) );
		m_lastScaling = m_pEntity->GetScaling();
	}

	if (m_bAttached)
		ToImpl(*m_pScene->GetScene())->addActor(*ToImpl(*m_pActor));
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

	ToImpl(*m_pScene->GetScene())->addActor(*ToImpl(*m_pActor));
}

// Detaches this controller from the scene.
void RigidStaticController::Detach()
{
	if (!m_bAttached)
		return;

	ToImpl(*m_pScene->GetScene())->removeActor(*ToImpl(*m_pActor));

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bAttached = false;
}

// Gets the number of child components.
uint4 RigidStaticController::GetComponentCount() const
{
	return 1;
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string RigidStaticController::GetComponentName(uint4 idx) const
{
	return "Material";
}

// Gets the n-th reflected child component, nullptr if not reflected.
const beCore::ReflectedComponent* RigidStaticController::GetReflectedComponent(uint4 idx) const
{
	return GetMaterial();
}

// Gets the type of the n-th child component.
beCore::Exchange::utf8_string RigidStaticController::GetComponentType(uint4 idx) const
{
	return "PhysicsMaterial";
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> RigidStaticController::GetComponent(uint4 idx) const
{
	return lean::any_value<Material*>( const_cast<Material*>( GetMaterial() ) );
}

// Returns true, if the n-th component can be replaced.
bool RigidStaticController::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void RigidStaticController::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetMaterial( any_cast<Material*>(pComponent) );
}

// Gets the controller type.
utf8_ntr RigidStaticController::GetControllerType()
{
	return utf8_ntr("RigidStaticController");
}

} // namespace

#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"

namespace bePhysics
{

/// Gets the default material for static rigid actors.
Material* GetRigidStaticDefaultMaterial(ResourceManager &resources)
{
	Material *material = resources.MaterialCache()->GetByName("RigidStaticController.Material");

	if (!material)
		material = resources.MaterialCache()->Set(
				CreateMaterial(*resources.MaterialCache()->GetDevice(), 0.9f, 0.8f, 0.05f, resources.MaterialCache()).get(),
				"RigidStaticController.Material"
			);

	return material;
}

} // namespace
