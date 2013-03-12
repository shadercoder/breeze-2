/*****************************************************/
/* breeze Engine Assets Module  (c) Tobias Zirr 2013 */
/*****************************************************/

#pragma once
#ifndef BE_ASSETS_WATER_CONTROLLERS
#define BE_ASSETS_WATER_CONTROLLERS

#include "beAssets.h"
#include <lean/pimpl/static_pimpl.h>
#include <beCore/beShared.h>
#include <beCore/beMany.h>
#include <beCore/beComponentMonitor.h>
#include <beScene/beRenderable.h>
#include <beEntitySystem/beEntityController.h>
#include <beEntitySystem/beSimulationController.h>

#include <beScene/beRenderingPipeline.h>
#include <beScene/bePerspectivePool.h>
#include <beScene/beRenderableMaterial.h>

#include <lean/smart/scoped_ptr.h>
#include <lean/smart/resource_ptr.h>

#include <beMath/beAABDef.h>

namespace beAssets
{

class WaterController;
class WaterControllers;

/// Handle to a water controller.
struct WaterControllerHandle : public beCore::GroupElementHandle<WaterControllers>
{
	friend WaterControllers;

private:
	/// Internal constructor.
	WaterControllerHandle(WaterControllers *controllers, uint4 internalID)
		: GroupElementHandle<WaterControllers>(controllers, internalID) { }
};

/// Water controller manager.
class LEAN_INTERFACE WaterControllers : public beEntitySystem::WorldController, public beScene::Renderable
{
	LEAN_SHARED_SIMPL_INTERFACE_BEHAVIOR(WaterControllers)

public:
	class M;

	/// Adds a controller.
	BE_ASSETS_API WaterController* AddController();
	/// Clones the given controller.
	BE_ASSETS_API static WaterController* CloneController(const WaterControllerHandle controller);
	/// Removes a controller.
	BE_ASSETS_API static void RemoveController(WaterController *pController);

	/// Commits changes.
	BE_ASSETS_API void Commit();
	
	/// Perform visiblity culling.
	BE_ASSETS_API void Cull(beScene::PipelinePerspective &perspective) const LEAN_OVERRIDE;
	/// Prepares the given render queue for the given perspective, returning true if active.
	BE_ASSETS_API bool Prepare(beScene::PipelinePerspective &perspective, beScene::PipelineQueueID queueID,
		const beScene::PipelineStageDesc &stageDesc, const beScene::RenderQueueDesc &queueDesc) const LEAN_OVERRIDE;
	/// Prepares the collected render queues for the given perspective.
	BE_ASSETS_API void Collect(beScene::PipelinePerspective &perspective) const LEAN_OVERRIDE;
	/// Performs optional optimization such as sorting.
	BE_ASSETS_API void Optimize(const beScene::PipelinePerspective &perspective, beScene::PipelineQueueID queueID) const LEAN_OVERRIDE;
	/// Prepares rendering from the collected render queues for the given perspective.
	BE_ASSETS_API void PreRender(const beScene::PipelinePerspective &perspective, const beScene::RenderContext &context) const LEAN_OVERRIDE;
	/// Renders the given render queue for the given perspective.
	BE_ASSETS_API void Render(const beScene::PipelinePerspective &perspective, beScene::PipelineQueueID queueID, const beScene::RenderContext &context) const LEAN_OVERRIDE;
	/// Renders the given single object for the given perspective.
	BE_ASSETS_API void Render(uint4 objectID, const beScene::PipelinePerspective &perspective, beScene::PipelineQueueID queueID, const beScene::RenderContext &context) const LEAN_OVERRIDE;

	/// Attaches the controller to the given entity.
	BE_ASSETS_API static void Attach(WaterControllerHandle controller, beEntitySystem::Entity *entity);
	/// Detaches the controller from the given entity.
	BE_ASSETS_API static void Detach(WaterControllerHandle controller, beEntitySystem::Entity *entity);

	/// Sets the material.
	BE_ASSETS_API static void SetMaterial(WaterControllerHandle controller, beScene::RenderableMaterial *pMaterial);
	/// Gets the material.
	BE_ASSETS_API static beScene::RenderableMaterial* GetMaterial(const WaterControllerHandle controller);

	/// Enables reflections.
	BE_ASSETS_API static void EnableReflection(WaterControllerHandle controller, bool bEnable);
	/// Checks if this light is currently reflecting.
	BE_ASSETS_API static bool IsReflectionEnabled(const WaterControllerHandle controller);

	/// Sets the reflection resolution.
	BE_ASSETS_API static void SetReflectionResolution(WaterControllerHandle controller, uint4 resolution);
	/// Gets the reflection resolution.
	BE_ASSETS_API static uint4 GetReflectionResolution(const WaterControllerHandle controller);

	/// Sets the reflection clip tolerance.
	BE_ASSETS_API static void SetReflectionTolerance(WaterControllerHandle controller, float tolerance);
	/// Gets the reflection clip tolerance.
	BE_ASSETS_API static float GetReflectionTolerance(const WaterControllerHandle controller);

	/// Sets the visibility.
	BE_ASSETS_API static void SetVisible(WaterControllerHandle controller, bool bVisible);
	/// Gets the visibility.
	BE_ASSETS_API static bool IsVisible(const WaterControllerHandle controller);
	
	/// Sets the local bounding sphere.
	BE_ASSETS_API static void SetLocalBounds(WaterControllerHandle controller, const beMath::faab3 &bounds);
	/// Gets the local bounding sphere.
	BE_ASSETS_API static const beMath::faab3& GetLocalBounds(const WaterControllerHandle controller);
	
	/// Sets the component monitor.
	BE_ASSETS_API void SetComponentMonitor(beCore::ComponentMonitor *componentMonitor);
	/// Gets the component monitor.
	BE_ASSETS_API beCore::ComponentMonitor* GetComponentMonitor() const;

	/// Gets the controller type.
	BE_ASSETS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_ASSETS_API const beCore::ComponentType* GetType() const;
};

/// Water controller.
class WaterController : public lean::noncopyable, public beEntitySystem::EntityController
{
public:
	friend WaterControllers;

protected:
	WaterControllerHandle m_handle;

	/// Internal constructor.
	WaterController(WaterControllerHandle handle)
		: m_handle(handle) { }

public:
	/// Synchronizes this controller with the given entity controlled.
	BE_ASSETS_API void Flush(const beEntitySystem::EntityHandle entity);

	/// Sets the material.
	LEAN_INLINE void SetMaterial(beScene::RenderableMaterial *pMaterial) { WaterControllers::SetMaterial(m_handle, pMaterial); }
	/// Gets the material.
	LEAN_INLINE beScene::RenderableMaterial* GetMaterial() const { return WaterControllers::GetMaterial(m_handle); }

	/// Enables reflection.
	LEAN_INLINE void EnableReflection(bool bEnable) { WaterControllers::EnableReflection(m_handle, bEnable); }
	/// Checks if this light is currently reflecting.
	LEAN_INLINE bool IsReflectionEnabled() const { return WaterControllers::IsReflectionEnabled(m_handle); }

	/// Sets the reflection resolution.
	LEAN_INLINE void SetReflectionResolution(uint4 resolution) { WaterControllers::SetReflectionResolution(m_handle, resolution); }
	/// Gets the reflection resolution.
	LEAN_INLINE uint4 GetReflectionResolution() const { return WaterControllers::GetReflectionResolution(m_handle); }

	/// Sets the reflection clip tolerance.
	LEAN_INLINE void SetReflectionTolerance(float tolerance) { WaterControllers::SetReflectionTolerance(m_handle, tolerance); }
	/// Gets the reflection clip tolerance.
	LEAN_INLINE float GetReflectionTolerance() const { return WaterControllers::GetReflectionTolerance(m_handle); }

	/// Sets the visibility.
	LEAN_INLINE void SetVisible(bool bVisible) { WaterControllers::SetVisible(m_handle, bVisible); }
	/// Gets the visibility.
	LEAN_INLINE bool IsVisible() const { return WaterControllers::IsVisible(m_handle); }

	/// Sets the local bounding sphere.
	LEAN_INLINE void SetLocalBounds(const beMath::faab3 &bounds) { WaterControllers::SetLocalBounds(m_handle, bounds); }
	/// Gets the local bounding sphere.
	LEAN_INLINE const beMath::faab3& GetLocalBounds() { return WaterControllers::GetLocalBounds(m_handle); }

	/// Attaches the entity.
	LEAN_INLINE void Attach(beEntitySystem::Entity *entity) { WaterControllers::Attach(m_handle, entity); }
	/// Detaches the entity.
	LEAN_INLINE void Detach(beEntitySystem::Entity *entity) { WaterControllers::Detach(m_handle, entity); }
	
	/// Adds a property listener.
	BE_ASSETS_API void AddObserver(beCore::ComponentObserver *listener) LEAN_OVERRIDE;
	/// Removes a property listener.
	BE_ASSETS_API void RemoveObserver(beCore::ComponentObserver *pListener) LEAN_OVERRIDE;

	/// Gets the reflection properties.
	BE_ASSETS_API static Properties GetOwnProperties();
	/// Gets the reflection properties.
	BE_ASSETS_API Properties GetReflectionProperties() const;

	/// Gets the number of child components.
	BE_ASSETS_API uint4 GetComponentCount() const;
	/// Gets the name of the n-th child component.
	BE_ASSETS_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_ASSETS_API lean::com_ptr<const ReflectedComponent, lean::critical_ref> GetReflectedComponent(uint4 idx) const;

	/// Gets the type of the n-th child component.
	BE_ASSETS_API const beCore::ComponentType* GetComponentType(uint4 idx) const;
	/// Gets the n-th component.
	BE_ASSETS_API lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const;
	/// Returns true, if the n-th component can be replaced.
	BE_ASSETS_API bool IsComponentReplaceable(uint4 idx) const;
	/// Sets the n-th component.
	BE_ASSETS_API void SetComponent(uint4 idx, const lean::any &pComponent);

	/// Gets the controller type.
	BE_ASSETS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_ASSETS_API const beCore::ComponentType* GetType() const;
	
	/// Clones this entity controller.
	BE_ASSETS_API WaterController* Clone() const { return WaterControllers::CloneController(m_handle); }
	/// Removes this controller.
	BE_ASSETS_API void Abandon() const { WaterControllers::RemoveController(const_cast<WaterController*>(this)); }

	/// Gets the handle to the entity.
	LEAN_INLINE WaterControllerHandle& Handle() { return m_handle; }
	/// Gets the handle to the entity.
	LEAN_INLINE const WaterControllerHandle& Handle() const { return m_handle; }
};

/// Creates a collection of water controllers.
/// @relatesalso WaterControllers
BE_ASSETS_API lean::scoped_ptr<WaterControllers, lean::critical_ref> CreateWaterControllers(beCore::PersistentIDs *persistentIDs,
	beScene::PerspectivePool *perspectivePool, const beScene::RenderingPipeline &pipeline, const beGraphics::Device &device);

} // namespace

#endif