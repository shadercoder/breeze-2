/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_SCENE_PX
#define BE_PHYSICS_SCENE_PX

#include "bePhysics.h"
#include "../beScene.h"
#include <lean/tags/noncopyable.h>
#include <beCore/beWrapper.h>
#include "beAPI.h"
#include "beMath.h"
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Creates a physics scene.
BE_PHYSICS_PX_API physx::PxScene* CreateScene(physx::PxPhysics &physics, const physx::PxSceneDesc &desc);

/// Converts the given device description into a PhysX tolerance scale object.
inline physx::PxSceneDesc ToImpl(const SceneDesc &desc, const physx::PxTolerancesScale &scale,
	physx::pxtask::CpuDispatcher *pDefaultCPUDispatcher = nullptr,
	physx::pxtask::GpuDispatcher *pDefaultGPUDispatcher = nullptr)
{
	physx::PxSceneDesc descPX(scale);
	descPX.gravity = ToImpl(desc.Gravity);
	descPX.bounceThresholdVelocity = desc.BounceThresholdSpeed;
	descPX.contactCorrelationDistance = desc.ContactCorrelationDist;
	descPX.cpuDispatcher = pDefaultCPUDispatcher;
	descPX.gpuDispatcher = pDefaultGPUDispatcher;
	return descPX;
}

// Prototypes
class Device;

/// Physics scene implementation.
class ScenePX : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxScene, ScenePX> >, public Scene
{
private:
	scoped_pxptr_t<physx::PxScene>::t m_pScene;

public:
	/// Constructor.
	BE_PHYSICS_PX_API ScenePX(Device &device, const SceneDesc &desc);
	/// Constructor.
	BE_PHYSICS_PX_API ScenePX(physx::PxScene *pScene);
	/// Destructor.
	BE_PHYSICS_PX_API ~ScenePX();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PXImplementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxScene*const& GetInterface() { return m_pScene.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxScene*const& GetInterface() const { return m_pScene.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxScene*const& GetScene() { return m_pScene.get(); }
};

template <> struct ToImplementationPX<Scene> { typedef ScenePX Type; };

} // namespace

#endif