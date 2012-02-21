/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beDevice.h"
#include <beCore/beShared.h>
#include <lean/logging/log.h>
#include <lean/logging/errors.h>

#include <PxVisualDebuggerExt.h>

namespace bePhysics
{

// Creates a physics foundation object.
physx::PxFoundation* CreateFoundation()
{
	static struct DefaultAllocator : public physx::PxAllocatorCallback
	{
		void* allocate(size_t size, const char*, const char*, int)
		{
			return beCore::exchange_heap::allocate<16>(size);
		}

		void deallocate(void *ptr)
		{
			 beCore::exchange_heap::free<16>(ptr);
		}

	} defaultAllocator;

	static struct DefaultLogger : public physx::PxErrorCallback
	{
		void reportError(physx::PxErrorCode::Enum e, const char *message, const char *file, int line)
		{
			const char *errorCode = "unknown error";

			switch (e)
			{
			case physx::PxErrorCode::eINVALID_PARAMETER:
				errorCode = "invalid parameter";
				break;
			case physx::PxErrorCode::eINVALID_OPERATION:
				errorCode = "invalid operation";
				break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:
				errorCode = "out of memory";
				break;
			case physx::PxErrorCode::eDEBUG_INFO:
				errorCode = "info";
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
				errorCode = "warning";
				break;
			}

			LEAN_LOG(file << " (" << line << "): PhysX " << errorCode << ": " << message);
		}

	} defaultLogger;

	scoped_pxptr_t<physx::PxFoundation>::t pFoundation(
		PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocator, defaultLogger) );

	if (!pFoundation)
		LEAN_THROW_ERROR_MSG("physx::PxCreateFoundation()");

	return pFoundation.detatch();
}

// Creates a physics interface object.
physx::PxPhysics* CreatePhysics(physx::PxFoundation &foundation, const physx::PxTolerancesScale &scale)
{
	scoped_pxptr_t<physx::PxPhysics>::t pPhysics(
		PxCreatePhysics(PX_PHYSICS_VERSION, foundation, scale) );

	if (!pPhysics)
		LEAN_THROW_ERROR_MSG("physx::PxCreatePhysics()");
	
	if (!PxInitExtensions(*pPhysics))
		LEAN_THROW_ERROR_MSG("physx::PxInitExtensions()");

	physx::PxExtensionVisualDebugger::connect(
		pPhysics->getPvdConnectionManager(),
		"127.0.0.1", 5425,
		100);

	return pPhysics.detatch();
}

// Creates a CPU dispatcher.
physx::PxDefaultCpuDispatcher* CreateCPUDispatcher(uint4 threadCount)
{
	physx::PxDefaultCpuDispatcher *pDispatcher = physx::PxDefaultCpuDispatcherCreate(threadCount);

	if (!pDispatcher)
		LEAN_THROW_ERROR_MSG("physx::PxDefaultCpuDispatcherCreate()");

	return pDispatcher;
}

// Creates a CUDA context. Returns nullptr if unavailable.
physx::pxtask::CudaContextManager* CreateCudaContext(physx::PxPhysics &physics)
{
	physx::pxtask::CudaContextManager *pContext = physx::pxtask::createCudaContextManager(
			physics.getFoundation(),
			physx::pxtask::CudaContextManagerDesc(),
			physics.getProfileZoneManager()
		);

	// NOTE: CUDA is always optional
	if (!pContext)
		LEAN_LOG_ERROR_MSG("createCudaContextManager()");

	return pContext;
}

// Constructor.
DevicePX::DevicePX(const DeviceDesc &desc)
	: m_pFoundation( CreateFoundation() ),
	m_pPhysics( CreatePhysics( *m_pFoundation, ToToleranceScale(desc) ) ),
	m_pCPUDispatcher( CreateCPUDispatcher(desc.ThreadCount) ),
	m_pCudaContext( (desc.EnableGPU) ? CreateCudaContext(*m_pPhysics) : nullptr )
{
}

// Constructor.
DevicePX::DevicePX(physx::PxPhysics *pPhysics,
		physx::PxDefaultCpuDispatcher *pCPUDispatcher,
		physx::pxtask::CudaContextManager *pCudaContext)
	: m_pPhysics( LEAN_ASSERT_NOT_NULL(pPhysics) ),
	m_pCPUDispatcher( pCPUDispatcher ),
	m_pCudaContext( pCudaContext )
{
}

// Destructor.
DevicePX::~DevicePX()
{
	// ORDER: Release SDK FIRST
	m_pCudaContext = nullptr;
	m_pCPUDispatcher = nullptr;
	m_pPhysics = nullptr;

	for (memory_vector::iterator it = m_staticMemory.begin(); it != m_staticMemory.end(); ++it)
		beCore::exchange_heap::free(it->first, it->second);
}

// Frees the given memory block on destruction.
void DevicePX::FreeOnRelease(void *pStaticMemory, size_t alignment)
{
	if (pStaticMemory)
		m_staticMemory.push_back( memory_pair(pStaticMemory, alignment) );
}

} // namespace