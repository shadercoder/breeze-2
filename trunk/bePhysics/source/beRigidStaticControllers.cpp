/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidStaticControllers.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beRigidShape.h"

#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/PX3/beScene.h"

#include "bePhysics/PX3/beMath.h"

#include <beEntitySystem/beEntities.h>

#include <beCore/bePersistentIDs.h>

#include <beCore/bePooled.h>
#include <beCore/bePool.h>

#include <lean/containers/multi_vector.h>
#include <lean/containers/simple_vector.h>
#include <lean/memory/chunk_pool.h>

namespace bePhysics
{

BE_CORE_PUBLISH_COMPONENT(RigidStaticController)
BE_CORE_PUBLISH_COMPONENT(RigidStaticControllers)

class RigidStaticControllers::M : public RigidStaticControllers
{
public:
	lean::resource_ptr<PX3::Device> device;
	lean::resource_ptr<PX3::Scene> scene;

	// Controller data
	struct Record
	{
		RigidStaticController *Reflected;
		lean::resource_ptr<PX3::RigidStatic> Actor;
		lean::resource_ptr<PX3::RigidShape> Shape;
		uint8 PersistentID;

		Record(RigidStaticController *reflected, PX3::RigidStatic *actor)
			: Reflected(reflected),
			Actor(actor),
			PersistentID(-1) { }
	};

	struct Configuration
	{
		bem::fvec3 LastScaling;

		Configuration()
			: LastScaling(1.0f) { }
	};

	struct State
	{
		physx::PxRigidStatic *Actor;
		Configuration Config;

		State(physx::PxRigidStatic *actor)
			: Actor(actor) { }
	};

	enum record_tag { record };
	enum state_tag { state };
	
	typedef lean::chunk_pool<RigidStaticController, 128> handle_pool;
	handle_pool handles;

	typedef lean::multi_vector_t< lean::simple_vector_binder<lean::vector_policies::semipod> >::make<
			Record, record_tag,
			State, state_tag
		>::type controllers_t;
	controllers_t controllers;
	
	lean::resource_ptr<beCore::ComponentMonitor> pComponentMonitor;

	M(Device *device, Scene *scene)
		: device( ToImpl(LEAN_ASSERT_NOT_NULL(device)) ),
		scene( ToImpl(LEAN_ASSERT_NOT_NULL(scene)) ) { }

	/// Gets the number of child components.
	uint4 GetComponentCount() const
	{
		return static_cast<uint4>(controllers.size());
	}
	/// Gets the name of the n-th child component.
	beCore::Exchange::utf8_string GetComponentName(uint4 idx) const
	{
		return GetComponentType()->Name;
	}
	/// Gets the n-th reflected child component, nullptr if not reflected.
	lean::com_ptr<const ReflectedComponent, lean::critical_ref> GetReflectedComponent(uint4 idx) const
	{
		return (idx < controllers.size()) ? controllers[idx].Reflected : nullptr;
	}

	/// Fixes all controller handles to match the layout of the given controller vector.
	static void FixControllerHandles(controllers_t &controllers, uint4 internalIdx = 0)
	{
		// Fix subsequent handles
		for (; internalIdx < controllers.size(); ++internalIdx)
			controllers(record)[internalIdx].Reflected->Handle().SetIndex(internalIdx);
	}

	/// Verifies the given handle.
	friend LEAN_INLINE bool VerifyHandle(const M &m, const RigidStaticControllerHandle handle) { return handle.Index < m.controllers.size(); }
};

/// Creates a collection of mesh controllers.
lean::scoped_ptr<RigidStaticControllers, lean::critical_ref> CreateRigidStaticControllers(beCore::PersistentIDs *persistentIDs, Device *device, Scene *scene)
{
	return new_scoped RigidStaticControllers::M(device, scene);
}

namespace
{

void CommitExternalChanges(RigidStaticControllers::M &m, beCore::ComponentMonitor &monitor)
{
	LEAN_FREE_PIMPL(RigidStaticControllers);

	// TODO: WHAT?
	bool bHasChanges = monitor.Structure.HasChanged(RigidShape::GetComponentType()) ||
		monitor.Data.HasChanged(RigidShape::GetComponentType());

	if (monitor.Replacement.HasChanged(RigidShape::GetComponentType()))
	{
		uint4 controllerCount = (uint4) m.controllers.size();

		for (uint4 internalIdx = 0; internalIdx < controllerCount; ++internalIdx)
		{
			RigidShape *oldShape = m.controllers(M::record)[internalIdx].Shape;
			RigidShape *shape = bec::GetSuccessor(oldShape);

			if (shape != oldShape)
				m.controllers(M::record)[internalIdx].Reflected->SetShape(shape);
		}
	}
}

} // namespace

// Commits changes.
void RigidStaticControllers::Commit()
{
	LEAN_STATIC_PIMPL();

	if (m.pComponentMonitor)
		CommitExternalChanges(m, *m.pComponentMonitor);
}

// Sets the mesh.
void RigidStaticControllers::SetShape(RigidStaticControllerHandle controller, RigidShape *pShape)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	M::Record &record = m.controllers(M::record)[controller.Index];

	if (record.Shape != pShape)
	{
		record.Shape = ToImpl(pShape);
		// NOTE: Shapes are expected to be complete w/ materials
		PX3::SetShape(**record.Actor, *record.Shape, nullptr);
		m.controllers(M::state)[controller.Index].Config.LastScaling = 1.0f;
	}
}

// Gets the mesh.
RigidShape* RigidStaticControllers::GetShape(const RigidStaticControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.controllers(M::record)[controller.Index].Shape;
}

// Adds a controller
RigidStaticController* RigidStaticControllers::AddController()
{
	LEAN_STATIC_PIMPL();
	
	lean::resource_ptr<PX3::RigidStatic> actor = new_resource PX3::RigidStatic(*m.device);

	uint4 internalIdx = static_cast<uint4>(m.controllers.size());
	RigidStaticController *handle;
	try
	{ 
		handle = new(m.handles.allocate()) RigidStaticController( RigidStaticControllerHandle(&m, internalIdx) );
		m.controllers.push_back( M::Record(handle, actor), actor->Get() );
	}
	LEAN_ASSERT_NOEXCEPT

	return handle;
}

// Clones the given controller.
RigidStaticController* RigidStaticControllers::CloneController(const RigidStaticControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE(const_cast<RigidStaticControllerHandle&>(controller));

	lean::scoped_ptr<RigidStaticController> clone( m.AddController() );
	RigidStaticControllerHandle cloneHandle = clone->Handle();
	
	// NOTE: Clone state rather than reconstructing controller -> keep scaling, mass ...
	PX3::CloneShape(**m.controllers[cloneHandle.Index].Actor, **m.controllers[controller.Index].Actor);
	m.controllers[cloneHandle.Index].Shape = m.controllers[controller.Index].Shape;
	m.controllers(M::state)[cloneHandle.Index].Config = m.controllers(M::state)[controller.Index].Config;

	return clone.detach();
}

// Removes a controller.
void RigidStaticControllers::RemoveController(RigidStaticController *pController)
{
	if (!pController || !pController->Handle().Group)
		return;

	BE_STATIC_PIMPL_HANDLE(pController->Handle());
	uint4 internalIdx  = pController->Handle().Index;

	try
	{
		m.controllers.erase(internalIdx);
		m.handles.free(pController);
	}
	LEAN_ASSERT_NOEXCEPT

	// Fix subsequent handles
	M::FixControllerHandles(m.controllers, internalIdx);
}

// Attaches the controller to the given entity.
void RigidStaticControllers::Attach(RigidStaticControllerHandle controller, beEntitySystem::Entity *entity)
{
	BE_STATIC_PIMPL_HANDLE(controller);

	// NOTE: Put into right place straight away
	m.controllers[controller.Index].Reflected->Flush(entity->Handle());
	m.scene->Get()->addActor(*m.controllers(M::state)[controller.Index].Actor);
}

// Detaches the controller from the given entity.
void RigidStaticControllers::Detach(RigidStaticControllerHandle controller, beEntitySystem::Entity *entity)
{
	BE_STATIC_PIMPL_HANDLE(controller);

	m.scene->Get()->removeActor(*m.controllers(M::state)[controller.Index].Actor);
}

// Sets the component monitor.
void RigidStaticControllers::SetComponentMonitor(beCore::ComponentMonitor *componentMonitor)
{
	LEAN_STATIC_PIMPL();
	m.pComponentMonitor = componentMonitor;
}

// Gets the component monitor.
beCore::ComponentMonitor* RigidStaticControllers::GetComponentMonitor() const
{
	LEAN_STATIC_PIMPL_CONST();
	return m.pComponentMonitor;
}

// Synchronizes this controller with the given entity controlled.
void RigidStaticController::Flush(const beEntitySystem::EntityHandle entity)
{
	BE_FREE_STATIC_PIMPL_HANDLE(RigidStaticControllers, m_handle);
	M::State &state = m.controllers(M::state)[m_handle.Index];

	physx::PxScene *pScene = state.Actor->getScene();
	if (pScene) pScene->removeActor(*state.Actor);

	using beEntitySystem::Entities;
	const Entities::Transformation& entityTrafo = Entities::GetTransformation(entity);
	state.Actor->setGlobalPose( PX3::ToTransform(entityTrafo.Orientation, entityTrafo.Position) );

	if (state.Config.LastScaling != entityTrafo.Scaling)
	{
		// TODO: Restore shapes first?
		PX3::Scale( *state.Actor, PX3::ToAPI(entityTrafo.Scaling / state.Config.LastScaling) );
		state.Config.LastScaling = entityTrafo.Scaling;
	}

	if (pScene) pScene->addActor(*state.Actor);
}

// Gets the number of child components.
uint4 RigidStaticController::GetComponentCount() const
{
	return 1;
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string RigidStaticController::GetComponentName(uint4 idx) const
{
	return "Shape";
}

// Gets the n-th reflected child component, nullptr if not reflected.
lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> RigidStaticController::GetReflectedComponent(uint4 idx) const
{
	return GetShape();
}


// Gets the type of the n-th child component.
const beCore::ComponentType* RigidStaticController::GetComponentType(uint4 idx) const
{
	return RigidShape::GetComponentType();
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> RigidStaticController::GetComponent(uint4 idx) const
{
	return bec::any_resource_t<RigidShape>::t( GetShape() );
}

// Returns true, if the n-th component can be replaced.
bool RigidStaticController::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void RigidStaticController::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetShape( any_cast<RigidShape*>(pComponent) );
}

} // namespace

#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"

namespace bePhysics
{

/// Gets the default material for static rigid actors.
Material* GetRigidStaticDefaultMaterial(ResourceManager &resources)
{
	Material *material = resources.MaterialCache()->GetByName("RigidStaticController.Material");

	if (!material)
	{
		lean::resource_ptr<Material> defaultMaterial = CreateMaterial(*resources.MaterialCache()->GetDevice(), 0.9f, 0.8f, 0.05f);
		resources.MaterialCache()->SetName(defaultMaterial, "RigidStaticController.Material");
		material = defaultMaterial;
	}

	return material;
}

} // namespace