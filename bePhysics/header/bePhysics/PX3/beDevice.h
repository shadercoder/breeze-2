/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_DEVICE_PX
#define BE_PHYSICS_DEVICE_PX

#include "bePhysics.h"
#include "../beDevice.h"
#include <lean/tags/noncopyable.h>
#include <beCore/beWrapper.h>
#include "beAPI.h"
#include <PxCudaContextManager.h>
#include <lean/smart/resource_ptr.h>
#include <vector>

namespace bePhysics
{

namespace PX3
{

/// Creates a physics foundation object.
BE_PHYSICS_PX_API physx::PxFoundation* CreateFoundation();
/// Creates a physics interface object.
BE_PHYSICS_PX_API physx::PxPhysics* CreatePhysics(physx::PxFoundation &foundation, const physx::PxTolerancesScale &scale = physx::PxTolerancesScale());
/// Creates a CPU dispatcher.
BE_PHYSICS_PX_API physx::PxDefaultCpuDispatcher* CreateCPUDispatcher(uint4 threadCount);
/// Creates a CUDA context. Returns nullptr if unavailable.
BE_PHYSICS_PX_API physx::pxtask::CudaContextManager* CreateCudaContext();

/// Allocates physx memory.
BE_PHYSICS_PX_API void* PhysXAllocate(size_t size);
/// Frees physx memory.
BE_PHYSICS_PX_API void PhysXFree(void *ptr);
/// Allocates physx memory (128-BYTE-aligned!).
BE_PHYSICS_PX_API void* PhysXSerializationAllocate(size_t size);
/// Frees physx memory.
BE_PHYSICS_PX_API void PhysXSerializationFree(void *ptr);

/// Converts the given device description into a PhysX tolerance scale object.
inline physx::PxTolerancesScale ToToleranceScale(const DeviceDesc &desc)
{
	physx::PxTolerancesScale scale;
	scale.length = desc.LengthScale;
	scale.mass = desc.MassScale;
	scale.speed = desc.SpeedScale;
	return scale;
}

/// Physics device implementation.
class Device : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxPhysics, Device> >, public bePhysics::Device
{
private:
	scoped_pxptr_t<physx::PxFoundation>::t m_pFoundation;
	scoped_pxptr_t<physx::PxPhysics>::t m_pPhysics;

	scoped_pxptr_t<physx::PxDefaultCpuDispatcher>::t m_pCPUDispatcher;
	scoped_pxptr_t<physx::pxtask::CudaContextManager>::t m_pCudaContext;

	typedef std::pair<void*, size_t> memory_pair;
	typedef std::vector<memory_pair> memory_vector;
	memory_vector m_staticMemory;

public:
	/// Constructor.
	BE_PHYSICS_PX_API Device(const DeviceDesc &desc);
	/// Constructor.
	BE_PHYSICS_PX_API Device(physx::PxPhysics *pPhysics,
		physx::PxDefaultCpuDispatcher *pCPUDispatcher = nullptr,
		physx::pxtask::CudaContextManager *pCudaContext = nullptr);
	/// Destructor.
	BE_PHYSICS_PX_API ~Device();

	/// Frees the given memory block on destruction.
	BE_PHYSICS_PX_API void FreeOnRelease(void *pStaticMemory);
	/// Frees the given serialization memory block on destruction.
	BE_PHYSICS_PX_API void SerializationFreeOnRelease(void *pStaticMemory);

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxPhysics*const& GetInterface() { return m_pPhysics.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxPhysics*const& GetInterface() const { return m_pPhysics.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxPhysics*const& GetPhysics() { return m_pPhysics.get(); }

	/// Gets the PhysX CPU dispatcher.
	LEAN_INLINE physx::pxtask::CpuDispatcher* GetCPUDispatcher() { return m_pCPUDispatcher.get(); }
	/// Gets the PhysX GPU dispatcher.
	LEAN_INLINE physx::pxtask::GpuDispatcher* GetGPUDispatcher() { return (m_pCudaContext) ? m_pCudaContext->getGpuDispatcher() : nullptr; }
};

template <> struct ToImplementationPX<bePhysics::Device> { typedef Device Type; };

} // namespace

} // namespace

#endif