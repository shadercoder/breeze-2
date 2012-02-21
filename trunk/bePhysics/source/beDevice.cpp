/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beDevice.h"

namespace bePhysics
{

// Creates a physics interface object.
lean::resource_ptr<Device, true> CreateDevice(const DeviceDesc &desc)
{
	return lean::bind_resource( new DevicePX(desc) );
}

} // namespace