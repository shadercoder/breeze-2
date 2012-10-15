/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidDynamicController.h"
#include <beEntitySystem/beEntity.h>

#include "bePhysics/beSceneController.h"
#include "bePhysics/PX3/beScene.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beMath.h"

#include <PxExtensionsAPI.h>

#include <beCore/beReflectionProperties.h>
#include <beCore/bePersistentIDs.h>

namespace bePhysics
{

const beCore::ReflectionProperties ControllerProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<float>("mass", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&RigidDynamicController::SetMassAndInertia) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&RigidDynamicController::GetMass) )
	<< beCore::MakeReflectionProperty<bool>("kinematic", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&RigidDynamicController::SetKinematic) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&RigidDynamicController::IsKinematic) );

// Constructor.
RigidDynamicController::RigidDynamicController(beEntitySystem::Entity *pEntity, SceneController *pScene, RigidDynamic *pActor,
		const ShapeCompound *pShape, const Material *pMaterial)
	: EntityController(pEntity, pScene),
	m_bAttached(false),
	
	m_pActor( LEAN_ASSERT_NOT_NULL(pActor) ),
	m_pShape( pShape ),
	m_pMaterial( pMaterial ),
	m_bKinematic(false),

	m_lastScaling(1.0f)
{
	Synchronize();
}

// Destructor.
RigidDynamicController::~RigidDynamicController()
{
	// WARNING: Never forget, will crash otherwise
	Detach();
}

// Sets the material.
void RigidDynamicController::SetMaterial(const Material *pMaterial)
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

// Sets the velocity.
void RigidDynamicController::SetVelocity(const beMath::fvec3 &v)
{
	ToImpl(*m_pActor)->setLinearVelocity( PX3::ToAPI(v) );
}

// Gets the velocity.
beMath::fvec3 RigidDynamicController::GetVelocity() const
{
	return PX3::FromAPI( PX3::ToImpl(*m_pActor)->getLinearVelocity() );
}

// Sets the mass.
void RigidDynamicController::SetMass(float mass)
{
	ToImpl(*m_pActor)->setMass( mass );
}

// Sets the mass.
void RigidDynamicController::SetMassAndInertia(float mass)
{
	physx::PxRigidBodyExt::setMassAndUpdateInertia( *ToImpl(*m_pActor), mass );
}

// Sets the mass.
float RigidDynamicController::GetMass() const
{
	return ToImpl(*m_pActor)->getMass();
}

// Forces this controller into synchronization with the simulation.
void RigidDynamicController::Synchronize()
{
	ToImpl(*m_pActor)->setGlobalPose( PX3::ToTransform(m_pEntity->GetOrientation(), m_pEntity->GetPosition()) );

	if (m_lastScaling != m_pEntity->GetScaling())
	{
		PX3::Scale( *ToImpl(*m_pActor), PX3::ToAPI(m_pEntity->GetScaling() / m_lastScaling) );
		m_lastScaling = m_pEntity->GetScaling();
		// Update inertia
		SetMassAndInertia(GetMass());
	}
}

// Synchronizes this controller with the simulation.
void RigidDynamicController::Flush()
{
	ToImpl(*m_pActor)->setKinematicTarget( PX3::ToTransform(m_pEntity->GetOrientation(), m_pEntity->GetPosition()) );
}

// Synchronizes the simulation with this controller.
void RigidDynamicController::Fetch()
{
	physx::PxTransform pose = ToImpl(*m_pActor)->getGlobalPose();
	m_pEntity->SetPosition( PX3::FromAPI(pose.p) );
	m_pEntity->SetOrientation( PX3::FromAPI(physx::PxMat33(pose.q)) );
}

// Converts the controlled actor between non-kinematic and kinematic.
void RigidDynamicController::SetKinematic(bool bKinematic)
{
	if (bKinematic != m_bKinematic)
	{
		bool bWasAttached = m_bAttached;

		if (bWasAttached)
			Detach();

		ToImpl(*m_pActor)->setRigidDynamicFlags((bKinematic) ? physx::PxRigidDynamicFlag::eKINEMATIC : physx::PxRigidDynamicFlags());
		m_bKinematic = bKinematic;

		if (bWasAttached)
			Attach();
	}
}

// Attaches this controller to the scene.
void RigidDynamicController::Attach()
{
	if (m_bAttached)
		return;

	// ORDER: Active as soon as SOMETHING MIGHT have been attached
	m_bAttached = true;

	Synchronize();

	ToImpl(*m_pScene->GetScene())->addActor(*ToImpl(*m_pActor));
	m_pScene->AddSynchronized(
		this,
		(m_bKinematic) ? beEntitySystem::SynchronizedFlags::Flush : beEntitySystem::SynchronizedFlags::Fetch);
}

// Detaches this controller from the scene.
void RigidDynamicController::Detach()
{
	if (!m_bAttached)
		return;

	ToImpl(*m_pScene->GetScene())->removeActor(*ToImpl(*m_pActor));
	m_pScene->RemoveSynchronized(this, beEntitySystem::SynchronizedFlags::All);

	// ORDER: Active as long as ANYTHING MIGHT be attached
	m_bAttached = false;
}

// Gets the number of child components.
uint4 RigidDynamicController::GetComponentCount() const
{
	return 1;
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string RigidDynamicController::GetComponentName(uint4 idx) const
{
	return "Material";
}

// Gets the n-th reflected child component, nullptr if not reflected.
const beCore::ReflectedComponent* RigidDynamicController::GetReflectedComponent(uint4 idx) const
{
	return GetMaterial();
}

// Gets the type of the n-th child component.
beCore::Exchange::utf8_string RigidDynamicController::GetComponentType(uint4 idx) const
{
	return "PhysicsMaterial";
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> RigidDynamicController::GetComponent(uint4 idx) const
{
	return lean::any_value<Material*>( const_cast<Material*>( GetMaterial() ) );
}

// Returns true, if the n-th component can be replaced.
bool RigidDynamicController::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void RigidDynamicController::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetMaterial( any_cast<Material*>(pComponent) );
}

// Gets the controller type.
utf8_ntr RigidDynamicController::GetControllerType()
{
	return utf8_ntr("RigidDynamicController");
}

// Gets the reflection properties.
RigidDynamicController::Properties RigidDynamicController::GetControllerProperties()
{
	return Properties(ControllerProperties.data(), ControllerProperties.data_end());
}

// Gets the reflection properties.
RigidDynamicController::Properties RigidDynamicController::GetReflectionProperties() const
{
	return Properties(ControllerProperties.data(), ControllerProperties.data_end());
}

} // namespace

#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"

namespace bePhysics
{

/// Gets the default material for static rigid actors.
Material* GetRigidDynamicDefaultMaterial(ResourceManager &resources)
{
	Material *material = resources.MaterialCache()->GetByName("RigidDynamicController.Material");

	if (!material)
		material = resources.MaterialCache()->Set(
				CreateMaterial(*resources.MaterialCache()->GetDevice(), 0.8f, 0.7f, 0.1f, resources.MaterialCache()).get(),
				"RigidDynamicController.Material"
			);

	return material;
}

} // namespace
