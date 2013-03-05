/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidDynamicControllers.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beRigidShape.h"

#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/PX3/beScene.h"

#include <PxExtensionsAPI.h>
#include "bePhysics/PX3/beMath.h"

#include <beEntitySystem/beEntities.h>

#include <beCore/bePersistentIDs.h>
#include <beCore/beReflectionProperties.h>

#include <beCore/bePooled.h>
#include <beCore/bePool.h>

#include <lean/containers/multi_vector.h>
#include <lean/containers/simple_vector.h>
#include <lean/memory/chunk_pool.h>

namespace bePhysics
{

BE_CORE_PUBLISH_COMPONENT(RigidDynamicController)
BE_CORE_PUBLISH_COMPONENT(RigidDynamicControllers)

const beCore::ReflectionProperty ControllerProperties[] =
{
	beCore::MakeReflectionProperty<float>("mass", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&RigidDynamicController::SetMass) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&RigidDynamicController::GetMass) ),
	beCore::MakeReflectionProperty<bool>("kinematic", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&RigidDynamicController::SetKinematic) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&RigidDynamicController::IsKinematic) )
};
BE_CORE_ASSOCIATE_PROPERTIES(RigidDynamicController, ControllerProperties)

class RigidDynamicControllers::M : public RigidDynamicControllers
{
public:
	lean::resource_ptr<PX3::Device> device;
	lean::resource_ptr<PX3::Scene> scene;

	// Controller data
	struct Record
	{
		RigidDynamicController *Reflected;
		lean::resource_ptr<PX3::RigidDynamic> Actor;
		lean::resource_ptr<PX3::RigidShape> Shape;
		uint8 PersistentID;

		Record(RigidDynamicController *reflected, PX3::RigidDynamic *actor)
			: Reflected(reflected),
			Actor(actor),
			PersistentID(-1) { }
	};

	struct Binding
	{
		physx::PxRigidDynamic *Actor;
		beEntitySystem::Entity *Entity;

		Binding(physx::PxRigidDynamic *actor)
			: Actor(actor),
			Entity() { }
	};

	struct Configuration
	{
		bem::fvec3 LastScaling;
		bool bKinematic;

		Configuration()
			: LastScaling(1.0f),
			bKinematic(false) { }
	};

	struct State
	{
		physx::PxRigidDynamic *Actor;
		Configuration Config;

		State(physx::PxRigidDynamic *actor)
			: Actor(actor) { }
	};

	enum record_tag { record };
	enum binding_tag { binding };
	enum state_tag { state };
	enum observers_tag { observers };
	
	typedef lean::chunk_pool<RigidDynamicController, 128> handle_pool;
	handle_pool handles;

	typedef lean::multi_vector_t< lean::simple_vector_binder<lean::vector_policies::semipod> >::make<
			Record, record_tag,
			Binding, binding_tag,
			State, state_tag,
			bec::ComponentObserverCollection, observers_tag
		>::type controllers_t;
	controllers_t controllers;
	bool bControllersChanged;
	
	lean::resource_ptr<beCore::ComponentMonitor> pComponentMonitor;

	M(Device *device, Scene *scene)
		: device( ToImpl(LEAN_ASSERT_NOT_NULL(device)) ),
		scene( ToImpl(LEAN_ASSERT_NOT_NULL(scene)) ),
		bControllersChanged( false ) { }

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
	friend LEAN_INLINE bool VerifyHandle(const M &m, const RigidDynamicControllerHandle handle) { return handle.Index < m.controllers.size(); }
};

/// Creates a collection of rigid actor controllers.
lean::scoped_ptr<RigidDynamicControllers, lean::critical_ref> CreateRigidDynamicControllers(beCore::PersistentIDs *persistentIDs, Device *device, Scene *scene)
{
	return new_scoped RigidDynamicControllers::M(device, scene);
}

namespace
{

void CommitExternalChanges(RigidDynamicControllers::M &m, beCore::ComponentMonitor &monitor)
{
	LEAN_FREE_PIMPL(RigidDynamicControllers);

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
void RigidDynamicControllers::Commit()
{
	LEAN_STATIC_PIMPL();

	if (m.pComponentMonitor)
		CommitExternalChanges(m, *m.pComponentMonitor);

	if (!m.bControllersChanged)
		return;

	// Update actor user data with internal IDs
	for (uint4 i = 0, count = (uint4) m.controllers.size(); i < count; ++i)
		m.controllers(M::binding)[i].Actor->userData = reinterpret_cast<void*>( (uintptr_t) i );

	m.bControllersChanged = false;
}

// Reads back the location of interaction physical objects.
void RigidDynamicControllers::Fetch()
{
	LEAN_STATIC_PIMPL();

	if (m.bControllersChanged)
		Commit();

	uint4 transformCount = 0;
	const physx::PxActiveTransform *transforms = m.scene()->Get()->getActiveTransforms(transformCount);

	uint4 controllerCount = (uint4) m.controllers.size();

	for (uint4 i = 0; i < transformCount; ++i)
	{
		const physx::PxActiveTransform &transform = transforms[i];
		
		// Check, if actor bound by this controller manager
		uint4 internalIdx = (uint4) reinterpret_cast<uintptr_t>(transform.userData);
		if (internalIdx < controllerCount)
		{
			const M::Binding &binding = m.controllers(M::binding)[internalIdx];
			if (binding.Actor == transform.actor)
			{
				bees::EntityHandle entity = binding.Entity->Handle();

				// Update bound entity
				bees::Entities::SetPosition( entity, PX3::FromAPI(transform.actor2World.p) );
				bees::Entities::SetOrientation( entity, PX3::FromAPI(physx::PxMat33(transform.actor2World.q)) );
			}
		}
	}
}

namespace
{

void PropertyChanged(RigidDynamicControllers::M &m, uint4 internalIdx)
{
	LEAN_FREE_PIMPL(RigidDynamicControllers);
	const bec::ComponentObserverCollection &observers = m.controllers(M::observers)[internalIdx];

	if (observers.HasObservers())
		observers.EmitPropertyChanged(*m.controllers(M::record)[internalIdx].Reflected);
}

} // namespace

// Sets the velocity.
void RigidDynamicControllers::SetVelocity(RigidDynamicControllerHandle controller, const bem::fvec3 &velocity)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	m.controllers(M::state)[controller.Index].Actor->setLinearVelocity( PX3::ToAPI(velocity) );
}

// Gets the velocity.
bem::fvec3 RigidDynamicControllers::GetVelocity(const RigidDynamicControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return PX3::FromAPI( m.controllers(M::state)[controller.Index].Actor->getLinearVelocity() );
}

// Sets this actor to behave kinematically.
void RigidDynamicControllers::SetKinematic(RigidDynamicControllerHandle controller, bool bKinematic)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	m.controllers(M::state)[controller.Index].Actor->setRigidDynamicFlags(
			bKinematic ? physx::PxRigidDynamicFlag::eKINEMATIC : physx::PxRigidDynamicFlags()
		);
	PropertyChanged(m, controller.Index);
}

// Gets whether this actore behaves kinematically.
bool RigidDynamicControllers::IsKinematic(const RigidDynamicControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.controllers(M::state)[controller.Index].Actor->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC;
}

// Sets the mass.
void RigidDynamicControllers::SetMass(RigidDynamicControllerHandle controller, float mass)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	physx::PxRigidBodyExt::setMassAndUpdateInertia(
			*m.controllers(M::state)[controller.Index].Actor,
			mass
		);
	PropertyChanged(m, controller.Index);
}

// Gets the mass.
float RigidDynamicControllers::GetMass(const RigidDynamicControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.controllers(M::state)[controller.Index].Actor->getMass();
}

// Sets the mesh.
void RigidDynamicControllers::SetShape(RigidDynamicControllerHandle controller, RigidShape *pShape)
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
RigidShape* RigidDynamicControllers::GetShape(const RigidDynamicControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.controllers(M::record)[controller.Index].Shape;
}

// Adds a controller
RigidDynamicController* RigidDynamicControllers::AddController()
{
	LEAN_STATIC_PIMPL();
	
	lean::resource_ptr<PX3::RigidDynamic> actor = new_resource PX3::RigidDynamic(*m.device);
	
	uint4 internalIdx = static_cast<uint4>(m.controllers.size());
	RigidDynamicController *handle;
	try
	{ 
		handle = new(m.handles.allocate()) RigidDynamicController( RigidDynamicControllerHandle(&m, internalIdx) );
		m.controllers.push_back( M::Record(handle, actor), actor->Get(), actor->Get() );
	}
	LEAN_ASSERT_NOEXCEPT

	m.bControllersChanged = true;

	return handle;
}

// Clones the given controller.
RigidDynamicController* RigidDynamicControllers::CloneController(const RigidDynamicControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE(const_cast<RigidDynamicControllerHandle&>(controller));

	lean::scoped_ptr<RigidDynamicController> clone( m.AddController() );
	RigidDynamicControllerHandle cloneHandle = clone->Handle();
	
	// NOTE: Clone state rather than reconstructing controller -> keep scaling, mass ...
	PX3::CloneShape(**m.controllers[cloneHandle.Index].Actor, **m.controllers[controller.Index].Actor);
	m.controllers[cloneHandle.Index].Shape = m.controllers[controller.Index].Shape;
	m.controllers(M::state)[cloneHandle.Index].Config = m.controllers(M::state)[controller.Index].Config;

	return clone.detach();
}

// Removes a controller.
void RigidDynamicControllers::RemoveController(RigidDynamicController *pController)
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

	m.bControllersChanged = true;

	// Fix subsequent handles
	M::FixControllerHandles(m.controllers, internalIdx);
}

// Attaches the controller to the given entity.
void RigidDynamicControllers::Attach(RigidDynamicControllerHandle controller, beEntitySystem::Entity *entity)
{
	BE_STATIC_PIMPL_HANDLE(controller);

	M::Binding &binding = m.controllers(M::binding)[controller.Index];
	binding.Entity = entity;
	// NOTE: Put into right place straight away
	m.controllers[controller.Index].Reflected->Synchronize(entity->Handle());
	m.scene->Get()->addActor(*binding.Actor);
}

// Detaches the controller from the given entity.
void RigidDynamicControllers::Detach(RigidDynamicControllerHandle controller, beEntitySystem::Entity *entity)
{
	BE_STATIC_PIMPL_HANDLE(controller);

	M::Binding &binding = m.controllers(M::binding)[controller.Index];
	binding.Entity = nullptr;
	m.scene->Get()->removeActor(*binding.Actor);
}

// Sets the component monitor.
void RigidDynamicControllers::SetComponentMonitor(beCore::ComponentMonitor *componentMonitor)
{
	LEAN_STATIC_PIMPL();
	m.pComponentMonitor = componentMonitor;
}

// Gets the component monitor.
beCore::ComponentMonitor* RigidDynamicControllers::GetComponentMonitor() const
{
	LEAN_STATIC_PIMPL_CONST();
	return m.pComponentMonitor;
}

// Adds a property listener.
void RigidDynamicController::AddObserver(beCore::ComponentObserver *listener)
{
	BE_FREE_STATIC_PIMPL_HANDLE(RigidDynamicControllers, m_handle);
	m.controllers(M::observers)[m_handle.Index].AddObserver(listener);
}

// Removes a property listener.
void RigidDynamicController::RemoveObserver(beCore::ComponentObserver *pListener)
{
	BE_FREE_STATIC_PIMPL_HANDLE(RigidDynamicControllers, m_handle);
	m.controllers(M::observers)[m_handle.Index].RemoveObserver(pListener);
}

// Synchronizes this controller with the given entity controlled.
void RigidDynamicController::Synchronize(beEntitySystem::EntityHandle entity)
{
	BE_FREE_STATIC_PIMPL_HANDLE(RigidDynamicControllers, m_handle);
	M::State &state = m.controllers(M::state)[m_handle.Index];

	using beEntitySystem::Entities;
	const Entities::Transformation& entityTrafo = Entities::GetTransformation(entity);
	state.Actor->setGlobalPose( PX3::ToTransform(entityTrafo.Orientation, entityTrafo.Position) );

	if (state.Config.LastScaling != entityTrafo.Scaling)
	{
		// TODO: Restore shapes first?
		PX3::Scale( *state.Actor, PX3::ToAPI(entityTrafo.Scaling / state.Config.LastScaling) );
		state.Config.LastScaling = entityTrafo.Scaling;
	}
}

// Synchronizes this controller with the given entity controlled.
void RigidDynamicController::Flush(const beEntitySystem::EntityHandle entity)
{
	BE_FREE_STATIC_PIMPL_HANDLE(RigidDynamicControllers, m_handle);
	const M::State &state = m.controllers(M::state)[m_handle.Index];

	if (state.Config.bKinematic)
	{
		using beEntitySystem::Entities;
		const Entities::Transformation& entityTrafo = Entities::GetTransformation(entity);
		state.Actor->setKinematicTarget( PX3::ToTransform(entityTrafo.Orientation, entityTrafo.Position) );
	}
}

// Gets the number of child components.
uint4 RigidDynamicController::GetComponentCount() const
{
	return 1;
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string RigidDynamicController::GetComponentName(uint4 idx) const
{
	return "Shape";
}

// Gets the n-th reflected child component, nullptr if not reflected.
lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> RigidDynamicController::GetReflectedComponent(uint4 idx) const
{
	return GetShape();
}


// Gets the type of the n-th child component.
const beCore::ComponentType* RigidDynamicController::GetComponentType(uint4 idx) const
{
	return RigidShape::GetComponentType();
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> RigidDynamicController::GetComponent(uint4 idx) const
{
	return bec::any_resource_t<RigidShape>::t( GetShape() );
}

// Returns true, if the n-th component can be replaced.
bool RigidDynamicController::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void RigidDynamicController::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetShape( any_cast<RigidShape*>(pComponent) );
}

} // namespace

#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"

namespace bePhysics
{

/// Gets the default material for static rigid actors.
Material* GetRigidDynamicDefaultMaterial(ResourceManager &resources)
{
	Material *material = resources.MaterialCache()->GetByName("RigidDynamicController.Material");

	if (!material)
	{
		lean::resource_ptr<Material> defaultMaterial = CreateMaterial(*resources.MaterialCache()->GetDevice(), 0.8f, 0.7f, 0.1f);
		resources.MaterialCache()->SetName(defaultMaterial, "RigidDynamicController.Material");
		material = defaultMaterial;
	}

	return material;
}

} // namespace