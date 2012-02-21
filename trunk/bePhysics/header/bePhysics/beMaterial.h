/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_MATERIAL
#define BE_PHYSICS_MATERIAL

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Material interface.
class Material : public beCore::Resource, public Implementation
{
protected:
	LEAN_INLINE Material& operator =(const Material&) { return *this; }

public:
	virtual ~Material() throw() { }

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
};

// Prototypes
class Device;

/// Creates a physics material.
BE_PHYSICS_API lean::resource_ptr<Material, true> CreateMaterial(Device &device, float staticFriction, float dynamicFriction, float restitution);

} // namespace

#endif