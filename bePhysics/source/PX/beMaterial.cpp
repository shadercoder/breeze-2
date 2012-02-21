/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beMaterial.h"
#include "bePhysics/PX/beDevice.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

// Creates a material.
physx::PxMaterial* CreateMaterial(physx::PxPhysics &physics, float staticFriction, float dynamicFriction, float restitution)
{
	physx::PxMaterial *pMaterial = physics.createMaterial(staticFriction, dynamicFriction, restitution);

	if (!pMaterial)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createMaterial()");

	return pMaterial;
}

// Constructor.
MaterialPX::MaterialPX(Device &device, float staticFriction, float dynamicFriction, float restitution)
	: m_pMaterial( CreateMaterial(*ToImpl(device), staticFriction, dynamicFriction, restitution) )
{
}

// Constructor.
MaterialPX::MaterialPX(physx::PxMaterial *pMaterial)
	: m_pMaterial( LEAN_ASSERT_NOT_NULL(pMaterial) )
{
}

// Destructor.
MaterialPX::~MaterialPX()
{
}

} // namespace