/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_RIGIDSTATICCONTROLLERS
#define BE_PHYSICS_RIGIDSTATICCONTROLLERS

#include "bePhysics.h"
#include <lean/pimpl/static_pimpl.h>
#include <beCore/beShared.h>
#include <beCore/beMany.h>
#include <beCore/beComponentMonitor.h>

#include <beEntitySystem/beEntityController.h>
#include <beEntitySystem/beSimulationController.h>
#include <lean/smart/scoped_ptr.h>

#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

// Prototypes
class RigidShape;
class Device;
class Scene;

class RigidStaticController;
class RigidStaticControllers;

/// Handle to a mesh controller.
struct RigidStaticControllerHandle : public beCore::GroupElementHandle<RigidStaticControllers>
{
	friend RigidStaticControllers;

private:
	/// Internal constructor.
	RigidStaticControllerHandle(RigidStaticControllers *controllers, uint4 internalID)
		: GroupElementHandle<RigidStaticControllers>(controllers, internalID) { }
};

/// Mesh controller manager.
class LEAN_INTERFACE RigidStaticControllers : public beCore::Resource, public beEntitySystem::WorldController
{
	LEAN_SHARED_SIMPL_INTERFACE_BEHAVIOR(RigidStaticControllers)

public:
	class M;
	
	/// Adds a rigid controller.
	BE_PHYSICS_API RigidStaticController* AddController();
	/// Clones the given controller.
	BE_PHYSICS_API static RigidStaticController* CloneController(const RigidStaticControllerHandle controller);
	/// Removes a rigid controller.
	BE_PHYSICS_API static void RemoveController(RigidStaticController *pController);

	/// Commits changes.
	BE_PHYSICS_API void Commit();

	/// Attaches the controller to the given entity.
	BE_PHYSICS_API static void Attach(RigidStaticControllerHandle controller, beEntitySystem::Entity *entity);
	/// Detaches the controller from the given entity.
	BE_PHYSICS_API static void Detach(RigidStaticControllerHandle controller, beEntitySystem::Entity *entity);

	/// Sets the mesh.
	BE_PHYSICS_API static void SetShape(RigidStaticControllerHandle controller, RigidShape *pShape);
	/// Gets the mesh.
	BE_PHYSICS_API static RigidShape* GetShape(const RigidStaticControllerHandle controller);

	/// Sets the component monitor.
	BE_PHYSICS_API void SetComponentMonitor(beCore::ComponentMonitor *componentMonitor);
	/// Gets the component monitor.
	BE_PHYSICS_API beCore::ComponentMonitor* GetComponentMonitor() const;

	/// Gets the controller type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const;
};

/// Rigid static controller.
class RigidStaticController : public lean::noncopyable, public beEntitySystem::EntityController
{
	friend RigidStaticControllers;

private:
	RigidStaticControllerHandle m_handle;

	/// Internal constructor.
	RigidStaticController(RigidStaticControllerHandle handle)
		: m_handle(handle) { }

public:
	/// Synchronizes this controller with the given entity controlled.
	BE_PHYSICS_API void Flush(const beEntitySystem::EntityHandle entity) LEAN_OVERRIDE;

	/// Sets the mesh.
	LEAN_INLINE void SetShape(RigidShape *pShape) { RigidStaticControllers::SetShape(m_handle, pShape); }
	/// Gets the mesh.
	LEAN_INLINE RigidShape* GetShape() const { return RigidStaticControllers::GetShape(m_handle); }

	/// Attaches the entity.
	BE_PHYSICS_API void Attach(beEntitySystem::Entity *entity) LEAN_OVERRIDE { RigidStaticControllers::Attach(m_handle, entity); }
	/// Detaches the entity.
	BE_PHYSICS_API void Detach(beEntitySystem::Entity *entity) LEAN_OVERRIDE { RigidStaticControllers::Detach(m_handle, entity); }

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

	/// Gets the controller type.
	BE_PHYSICS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_PHYSICS_API const beCore::ComponentType* GetType() const LEAN_OVERRIDE;
	
	/// Clones this entity controller.
	BE_PHYSICS_API RigidStaticController* Clone() const { return RigidStaticControllers::CloneController(m_handle); }
	/// Removes this controller.
	BE_PHYSICS_API void Abandon() const LEAN_OVERRIDE { RigidStaticControllers::RemoveController(const_cast<RigidStaticController*>(this)); }

	/// Gets the handle to the entity.
	LEAN_INLINE RigidStaticControllerHandle& Handle() { return m_handle; }
	/// Gets the handle to the entity.
	LEAN_INLINE const RigidStaticControllerHandle& Handle() const { return m_handle; }
};

/// Creates a collection of rigid static controllers.
/// @relatesalso RigidStaticControllers
BE_PHYSICS_API lean::scoped_ptr<RigidStaticControllers, lean::critical_ref> CreateRigidStaticControllers(beCore::PersistentIDs *persistentIDs,
																										 Device *device, Scene *scene);

class ResourceManager;
class Material;

/// Gets the default material for static rigid actors.
BE_PHYSICS_API Material* GetRigidStaticDefaultMaterial(ResourceManager &resources);

} // namespace

#endif