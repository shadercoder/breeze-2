/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_MATERIAL
#define BE_PHYSICS_MATERIAL

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beCore/beManagedResource.h>
#include <lean/smart/resource_ptr.h>

#include <beCore/beReflectionPropertyProvider.h>

namespace bePhysics
{

class MaterialCache;

/// Material interface.
class LEAN_INTERFACE Material : public beCore::Resource, public beCore::ReflectionPropertyProvider,
	public beCore::ManagedResource<MaterialCache>, public beCore::HotResource<Material>, public Implementation
{
	LEAN_SHARED_BASE_BEHAVIOR(Material)

public:
	/// Sets the static friction.
	virtual void SetStaticFriction(float friction) = 0;
	/// Gets the static friction.
	virtual float GetStaticFriction() const = 0;

	/// Sets the dynamic friction.
	virtual void SetDynamicFriction(float friction) = 0;
	/// Gets the dynamic friction.
	virtual float GetDynamicFriction() const = 0;

	/// Sets the restitution.
	virtual void SetRestitution(float restitution) = 0;
	/// Gets the restitution.
	virtual float GetRestitution() const = 0;

	/// Gets the component type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the component type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const;
};

// Prototypes
class Device;

/// Creates a physics material.
BE_PHYSICS_API lean::resource_ptr<Material, true> CreateMaterial(Device &device, float staticFriction, float dynamicFriction, float restitution);

} // namespace

#endif