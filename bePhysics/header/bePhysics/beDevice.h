/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_DEVICE
#define BE_PHYSICS_DEVICE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Physics device description.
struct DeviceDesc
{
	float LengthScale;	///< Approximate size of objects in the simulation.
	float MassScale;	///< Approximate mass of unit objects in the simulation.
	float SpeedScale;	///< Approximate velocity of objects in the simulation.

	uint4 ThreadCount;	///< Number of worker threads.
	bool EnableGPU;		///< Enables GPU processing.

	/// Constructor.
	DeviceDesc(float length = 1.0f,
		float mass = 1000.0f,
		float speed = 10.0f,
		uint4 threadCount = 1,
		bool bEnableGPU = true)
			: LengthScale(length),
			MassScale(mass),
			SpeedScale(speed),
			ThreadCount(threadCount),
			EnableGPU(bEnableGPU) { }
};

/// Physics device interface.
class Device : public beCore::Resource, public Implementation
{
protected:
	LEAN_INLINE Device& operator =(const Device&) { return *this; }

public:
	virtual ~Device() throw() { }
};

/// Creates a physics device.
BE_PHYSICS_API lean::resource_ptr<Device, true> CreateDevice(const DeviceDesc &desc);

} // namespace

#endif