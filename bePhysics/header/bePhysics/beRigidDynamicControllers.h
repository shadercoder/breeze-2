/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_RIGIDDYNAMICCONTROLLERS
#define BE_PHYSICS_RIGIDDYNAMICCONTROLLERS

#include "bePhysics.h"
#include <lean/pimpl/static_pimpl.h>
#include <beCore/beShared.h>
#include <beCore/beMany.h>
#include <beCore/beComponentMonitor.h>

#include <beEntitySystem/beEntityController.h>
#include <beEntitySystem/beSynchronized.h>
#include <beEntitySystem/beSimulationController.h>
#include <lean/smart/scoped_ptr.h>

#include <lean/smart/resource_ptr.h>

#include <beMath/beVector.h>

namespace bePhysics
{

// Prototypes
class RigidShape;
class Device;
class Scene;

class RigidDynamicController;
class RigidDynamicControllers;

/// Handle to a mesh controller.
struct RigidDynamicControllerHandle : public beCore::GroupElementHandle<RigidDynamicControllers>
{
	friend RigidDynamicControllers;

private:
	/// Internal constructor.
	RigidDynamicControllerHandle(RigidDynamicControllers *controllers, uint4 internalID)
		: GroupElementHandle<RigidDynamicControllers>(controllers, internalID) { }
};

/// Rigid dynamic controller manager.
class LEAN_INTERFACE RigidDynamicControllers : public beCore::Resource, public beEntitySystem::WorldController,
	public beEntitySystem::Synchronized
{
	LEAN_SHARED_SIMPL_INTERFACE_BEHAVIOR(RigidDynamicControllers)

public:
	class M;
	
	/// Adds a rigid controller.
	BE_PHYSICS_API RigidDynamicController* AddController();
	/// Clones the given controller.
	BE_PHYSICS_API static RigidDynamicController* CloneController(const RigidDynamicControllerHandle controller);
	/// Removes a rigid controller.
	BE_PHYSICS_API static void RemoveController(RigidDynamicController *pController);

	/// Commits changes.
	BE_PHYSICS_API void Commit();

	/// Reads back the location of interaction physical objects.
	BE_PHYSICS_API void Fetch();

	/// Attaches the controller to the given entity.
	BE_PHYSICS_API static void Attach(RigidDynamicControllerHandle controller, beEntitySystem::Entity *entity);
	/// Detaches the controller from the given entity.
	BE_PHYSICS_API static void Detach(RigidDynamicControllerHandle controller, beEntitySystem::Entity *entity);

	/// Sets the mesh.
	BE_PHYSICS_API static void SetShape(RigidDynamicControllerHandle controller, RigidShape *pShape);
	/// Gets the mesh.
	BE_PHYSICS_API static RigidShape* GetShape(const RigidDynamicControllerHandle controller);

	/// Sets the velocity.
	BE_PHYSICS_API static void SetVelocity(RigidDynamicControllerHandle controller, const bem::fvec3 &velocity);
	/// Gets the velocity.
	BE_PHYSICS_API static bem::fvec3 GetVelocity(const RigidDynamicControllerHandle controller);

	/// Sets this actor to behave kinematically.
	BE_PHYSICS_API static void SetKinematic(RigidDynamicControllerHandle controller, bool bKinematic);
	/// Gets whether this actore behaves kinematically.
	BE_PHYSICS_API static bool IsKinematic(const RigidDynamicControllerHandle controller);

	/// Sets the mass.
	BE_PHYSICS_API static void SetMass(RigidDynamicControllerHandle controller, float mass);
	/// Gets the mass.
	BE_PHYSICS_API static float GetMass(const RigidDynamicControllerHandle controller);

	/// Sets the component monitor.
	BE_PHYSICS_API void SetComponentMonitor(beCore::ComponentMonitor *componentMonitor);
	/// Gets the component monitor.
	BE_PHYSICS_API beCore::ComponentMonitor* GetComponentMonitor() const;

	/// Gets the controller type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const;
};

/// Rigid dynamic controller.
class RigidDynamicController : public lean::noncopyable, public beEntitySystem::EntityController
{
	friend RigidDynamicControllers;

private:
	RigidDynamicControllerHandle m_handle;

	/// Internal constructor.
	RigidDynamicController(RigidDynamicControllerHandle handle)
		: m_handle(handle) { }

public:
	/// Synchronizes this controller with the given entity controlled.
	BE_PHYSICS_API void Synchronize(beEntitySystem::EntityHandle entity) LEAN_OVERRIDE;
	/// Synchronizes this controller with the given entity controlled.
	BE_PHYSICS_API void Flush(const beEntitySystem::EntityHandle entity) LEAN_OVERRIDE;

	/// Sets the mesh.
	LEAN_INLINE void SetShape(RigidShape *pShape) { RigidDynamicControllers::SetShape(m_handle, pShape); }
	/// Gets the mesh.
	LEAN_INLINE RigidShape* GetShape() const { return RigidDynamicControllers::GetShape(m_handle); }
	
	/// Sets the velocity.
	LEAN_INLINE void SetVelocity(const bem::fvec3 &velocity) { RigidDynamicControllers::SetVelocity(m_handle, velocity); }
	/// Gets the velocity.
	LEAN_INLINE bem::fvec3 GetVelocity() const { return RigidDynamicControllers::GetVelocity(m_handle); }

	/// Sets this actor to behave kinematically.
	LEAN_INLINE void SetKinematic(bool bKinematic) { RigidDynamicControllers::SetKinematic(m_handle, bKinematic); }
	/// Gets whether this actore behaves kinematically.
	LEAN_INLINE bool IsKinematic() const { return RigidDynamicControllers::IsKinematic(m_handle); }

	/// Sets the mass.
	LEAN_INLINE void SetMass(float mass) { RigidDynamicControllers::SetMass(m_handle, mass); }
	/// Gets the mass.
	LEAN_INLINE float GetMass() const { return RigidDynamicControllers::GetMass(m_handle); }

	/// Attaches the entity.
	BE_PHYSICS_API void Attach(beEntitySystem::Entity *entity) LEAN_OVERRIDE { RigidDynamicControllers::Attach(m_handle, entity); }
	/// Detaches the entity.
	BE_PHYSICS_API void Detach(beEntitySystem::Entity *entity) LEAN_OVERRIDE { RigidDynamicControllers::Detach(m_handle, entity); }

	/// Gets the number of child components.
	BE_PHYSICS_API uint4 GetComponentCount() const LEAN_OVERRIDE;
	/// Gets the name of the n-th child component.
	BE_PHYSICS_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const LEAN_OVERRIDE;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_PHYSICS_API lean::com_ptr<const ReflectedComponent, lean::critical_ref> GetReflectedComponent(uint4 idx) const LEAN_OVERRIDE;

	/// Gets the type of the n-th child component.
	BE_PHYSICS_API const beCore::ComponentType* GetComponentType(uint4 idx) const LEAN_OVERRIDE;
	/// Gets the n-th component.
	BE_PHYSICS_API lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const LEAN_OVERRIDE;
	/// Returns true, if the n-th component can be replaced.
	BE_PHYSICS_API bool IsComponentReplaceable(uint4 idx) const LEAN_OVERRIDE;
	/// Sets the n-th component.
	BE_PHYSICS_API void SetComponent(uint4 idx, const lean::any &pComponent) LEAN_OVERRIDE;
	
	/// Adds a property listener.
	BE_PHYSICS_API void AddObserver(beCore::ComponentObserver *listener) LEAN_OVERRIDE;
	/// Removes a property listener.
	BE_PHYSICS_API void RemoveObserver(beCore::ComponentObserver *pListener) LEAN_OVERRIDE;

	/// Gets the reflection properties.
	BE_PHYSICS_API static Properties GetOwnProperties();
	/// Gets the reflection properties.
	BE_PHYSICS_API Properties GetReflectionProperties() const;

	/// Gets the controller type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const LEAN_OVERRIDE;
	
	/// Clones this entity controller.
	BE_PHYSICS_API RigidDynamicController* Clone() const { return RigidDynamicControllers::CloneController(m_handle); }
	/// Removes this controller.
	BE_PHYSICS_API void Abandon() const LEAN_OVERRIDE { RigidDynamicControllers::RemoveController(const_cast<RigidDynamicController*>(this)); }

	/// Gets the handle to the entity.
	LEAN_INLINE RigidDynamicControllerHandle& Handle() { return m_handle; }
	/// Gets the handle to the entity.
	LEAN_INLINE const RigidDynamicControllerHandle& Handle() const { return m_handle; }
};

/// Creates a collection of rigid dynamic controllers.
/// @relatesalso RigidDynamicControllers
BE_PHYSICS_API lean::scoped_ptr<RigidDynamicControllers, lean::critical_ref> CreateRigidDynamicControllers(beCore::PersistentIDs *persistentIDs,
																										   Device *device, Scene *scene);

class ResourceManager;
class Material;

/// Gets the default material for dynamic rigid actors.
BE_PHYSICS_API Material* GetRigidDynamicDefaultMaterial(ResourceManager &resources);

} // namespace

#endif