/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_MATERIAL_PX
#define BE_PHYSICS_MATERIAL_PX

#include "bePhysics.h"
#include "../beMaterial.h"
#include <lean/tags/noncopyable.h>
#include <beCore/beWrapper.h>
#include "beAPI.h"
#include "beMath.h"
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Creates a material.
BE_PHYSICS_PX_API physx::PxMaterial* CreateMaterial(physx::PxPhysics &physics, float staticFriction, float dynamicFriction, float restitution);

// Prototypes
class Device;

/// Physics scene implementation.
class MaterialPX : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxMaterial, MaterialPX> >, public Material
{
private:
	scoped_pxptr_t<physx::PxMaterial>::t m_pMaterial;

public:
	/// Constructor.
	BE_PHYSICS_PX_API MaterialPX(Device &device, float staticFriction, float dynamicFriction, float restitution);
	/// Constructor.
	BE_PHYSICS_PX_API MaterialPX(physx::PxMaterial *pScene);
	/// Destructor.
	BE_PHYSICS_PX_API ~MaterialPX();

	/// Sets the static friction.
	void SetStaticFriction(float friction) { m_pMaterial->setStaticFriction(friction); }
	/// Gets the static friction.
	float GetStaticFriction() const { return m_pMaterial->getStaticFriction(); }

	/// Sets the dynamic friction.
	void SetDynamicFriction(float friction) { m_pMaterial->setDynamicFriction(friction); }
	/// Gets the dynamic friction.
	float GetDynamicFriction() const { return m_pMaterial->getDynamicFriction(); }

	/// Sets the restitution.
	void SetRestitution(float restitution) { m_pMaterial->setRestitution(restitution); }
	/// Gets the restitution.
	float GetRestitution() const { return m_pMaterial->getRestitution(); }

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PXImplementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxMaterial*const& GetInterface() { return m_pMaterial.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxMaterial*const& GetInterface() const { return m_pMaterial.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxMaterial*const& GetScene() { return m_pMaterial.get(); }
};

template <> struct ToImplementationPX<Material> { typedef MaterialPX Type; };

} // namespace

#endif