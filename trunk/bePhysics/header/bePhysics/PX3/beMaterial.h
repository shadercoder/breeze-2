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

// Prototypes
class Device;
class MaterialCache;

namespace PX3
{

/// Creates a material.
BE_PHYSICS_PX_API physx::PxMaterial* CreateMaterial(physx::PxPhysics &physics, float staticFriction, float dynamicFriction, float restitution);

/// Physics scene implementation.
class Material : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxMaterial, Material> >, public bePhysics::Material
{
private:
	scoped_pxptr_t<physx::PxMaterial>::t m_pMaterial;

	bePhysics::MaterialCache *m_pMaterialCache;

public:
	/// Constructor.
	BE_PHYSICS_PX_API Material(bePhysics::Device &device, float staticFriction, float dynamicFriction, float restitution,
		bePhysics::MaterialCache *pMaterialCache = nullptr);
	/// Constructor.
	BE_PHYSICS_PX_API Material(physx::PxMaterial *material,
		bePhysics::MaterialCache *pMaterialCache = nullptr);
	/// Destructor.
	BE_PHYSICS_PX_API ~Material();

	/// Sets the static friction.
	BE_PHYSICS_PX_API void SetStaticFriction(float friction) { m_pMaterial->setStaticFriction(friction); }
	/// Gets the static friction.
	BE_PHYSICS_PX_API float GetStaticFriction() const { return m_pMaterial->getStaticFriction(); }

	/// Sets the dynamic friction.
	BE_PHYSICS_PX_API void SetDynamicFriction(float friction) { m_pMaterial->setDynamicFriction(friction); }
	/// Gets the dynamic friction.
	BE_PHYSICS_PX_API float GetDynamicFriction() const { return m_pMaterial->getDynamicFriction(); }

	/// Sets the restitution.
	BE_PHYSICS_PX_API void SetRestitution(float restitution) { m_pMaterial->setRestitution(restitution); }
	/// Gets the restitution.
	BE_PHYSICS_PX_API float GetRestitution() const { return m_pMaterial->getRestitution(); }

	/// Gets the reflection properties.
	BE_PHYSICS_PX_API static Properties GetMaterialProperties();
	/// Gets the reflection properties.
	BE_PHYSICS_PX_API Properties GetReflectionProperties() const;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxMaterial*const& GetInterface() { return m_pMaterial.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxMaterial*const& GetInterface() const { return m_pMaterial.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxMaterial*const& GetScene() { return m_pMaterial.get(); }

	/// Gets the material cache.
	bePhysics::MaterialCache* GetCache() const { return m_pMaterialCache; };
};

template <> struct ToImplementationPX<bePhysics::Material> { typedef Material Type; };

} // namespace

} // namespace

#endif