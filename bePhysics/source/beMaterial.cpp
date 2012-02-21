/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beDevice.h"
#include "bePhysics/PX/beMaterial.h"

namespace bePhysics
{

// Creates a physics material.
lean::resource_ptr<Material, true> CreateMaterial(Device &device, float staticFriction, float dynamicFriction, float restitution)
{
	return lean::bind_resource<Material>( new MaterialPX(
		CreateMaterial(*ToImpl(device), staticFriction, dynamicFriction, restitution) ) );
}

} // namespace