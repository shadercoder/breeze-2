/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_RIGIDSTATICCONTROLLER
#define BE_PHYSICS_RIGIDSTATICCONTROLLER

#include "bePhysics.h"
#include "beEntityController.h"
#include <lean/smart/resource_ptr.h>
#include <beMath/beVectorDef.h>

namespace bePhysics
{

// Prototypes
class SceneController;
class RigidStatic;
class Material;
class ShapeCompound;

/// Static rigid body controller class.
class RigidStaticController : public EntityController
{
private:
	bool m_bAttached;

	beMath::fvec3 m_lastScaling;

protected:
	/// Shape
	lean::resource_ptr<const ShapeCompound> m_pShape;

	/// Material
	lean::resource_ptr<const Material> m_pMaterial;

	/// Actor.
	lean::resource_ptr<RigidStatic> m_pActor;

public:
	/// Constructor.
	BE_PHYSICS_API RigidStaticController(beEntitySystem::Entity *pEntity, SceneController *pScene, RigidStatic *pActor, 
		const ShapeCompound *pShape = nullptr, const Material *pMaterial = nullptr);
	/// Destructor.
	BE_PHYSICS_API ~RigidStaticController();

	/// Sets the material.
	BE_PHYSICS_API void SetMaterial(const Material *pMaterial);
	/// Gets the material.
	LEAN_INLINE const Material* GetMaterial() const { return m_pMaterial; }

	/// Forces this controller into synchronization with the simulation.
	BE_PHYSICS_API void Synchronize();

	/// Attaches this controller to the scene.
	BE_PHYSICS_API void Attach();
	/// Detaches this controller from the scene.
	BE_PHYSICS_API void Detach();

	/// Gets the number of child components.
	BE_PHYSICS_API uint4 GetComponentCount() const;
	/// Gets the name of the n-th child component.
	BE_PHYSICS_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_PHYSICS_API const ReflectedComponent* GetReflectedComponent(uint4 idx) const;

	/// Gets the type of the n-th child component.
	BE_PHYSICS_API beCore::Exchange::utf8_string GetComponentType(uint4 idx) const;
	/// Gets the n-th component.
	BE_PHYSICS_API lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const;
	/// Returns true, if the n-th component can be replaced.
	BE_PHYSICS_API bool IsComponentReplaceable(uint4 idx) const;
	/// Sets the n-th component.
	BE_PHYSICS_API void SetComponent(uint4 idx, const lean::any &pComponent);

	/// Gets the shape.
	LEAN_INLINE const ShapeCompound* GetShape() const { return m_pShape; }
	/// Gets the actor.
	LEAN_INLINE RigidStatic* GetActor() const { return m_pActor; }

	/// Gets the controller type.
	BE_PHYSICS_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

class ResourceManager;

/// Gets the default material for static rigid actors.
BE_PHYSICS_API Material* GetRigidStaticDefaultMaterial(ResourceManager &resources);

} // namespace

#endif