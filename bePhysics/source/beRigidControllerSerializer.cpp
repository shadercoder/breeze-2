/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidControllerSerializer.h"
#include "bePhysics/beRigidStaticControllers.h"
#include "bePhysics/beRigidDynamicControllers.h"

#include <beEntitySystem/beSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include "bePhysics/beSerializationParameters.h"

#include <beEntitySystem/beWorld.h>

#include <beCore/beParameterSet.h>
#include <beCore/beParameters.h>
#include <beCore/beExchangeContainers.h>

#include "bePhysics/beInlineMaterialSerialization.h"
#include "bePhysics/beInlineShapeSerialization.h"

#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"
#include "bePhysics/beShapeCache.h"

#include "bePhysics/beMaterial.h"
#include "bePhysics/beShapes.h"
#include "bePhysics/beRigidShape.h"

#include "bePhysics/beSceneController.h"

#include <lean/xml/numeric.h>

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace
{

template <class RigidController>
Material* GetDefaultMaterial(ResourceManager &resources);

template <>
Material* GetDefaultMaterial<RigidStaticController>(ResourceManager &resources)
{
	return GetRigidStaticDefaultMaterial(resources);
}

template <>
Material* GetDefaultMaterial<RigidDynamicController>(ResourceManager &resources)
{
	return GetRigidDynamicDefaultMaterial(resources);
}

template <class RigidController>
struct ActorFactory;

template <>
struct ActorFactory<RigidStaticController>
{
	typedef RigidStaticControllers Controllers;

	static lean::scoped_ptr<Controllers, lean::critical_ref> CreateControllers(beCore::PersistentIDs *persistentIDs, Device *device, Scene *scene)
	{
		return CreateRigidStaticControllers(persistentIDs, device, scene);
	}

	static void LinkControllers(Controllers *ctrl, SceneController &sceneCtrl) { }
};

template <>
struct ActorFactory<RigidDynamicController>
{
	typedef RigidDynamicControllers Controllers;
	
	static lean::scoped_ptr<Controllers, lean::critical_ref> CreateControllers(beCore::PersistentIDs *persistentIDs, Device *device, Scene *scene)
	{
		return CreateRigidDynamicControllers(persistentIDs, device, scene);
	}

	static void LinkControllers(Controllers *ctrl, SceneController &sceneCtrl)
	{
		sceneCtrl.AddSynchronized(ctrl, bees::SynchronizedFlags::All);
	}
};

template <class ControllerType>
typename ActorFactory<ControllerType>::Controllers* CreateRigidControllers(const beCore::ParameterSet &parameters)
{
	typedef typename ActorFactory<ControllerType>::Controllers Controllers;

	bees::EntitySystemParameters entityParameters = bees::GetEntitySystemParameters(parameters);
	bees::World *world = LEAN_THROW_NULL(entityParameters.World);

	Controllers *pControllers = static_cast<Controllers*>(
			world->Controllers().GetController(Controllers::GetComponentType())
		);
	
	if (!pControllers)
	{
		PhysicsParameters sceneParameters = GetPhysicsParameters(parameters);
		SceneController *sceneCtrl = LEAN_THROW_NULL(sceneParameters.SceneController);

		pControllers = ActorFactory<ControllerType>::CreateControllers(
				&world->PersistentIDs(),
				sceneParameters.Device,
				sceneParameters.Scene
			).detach();
		pControllers->SetComponentMonitor(sceneParameters.ResourceManager->Monitor());
		world->Controllers().AddControllerKeep(pControllers);
		ActorFactory<ControllerType>::LinkControllers(pControllers, *sceneCtrl);
	}

	return pControllers;
}

// Gets the serialization parameter IDs.
template <class ControllerType>
typename ActorFactory<ControllerType>::Controllers* GetRigidControllers(const beCore::ParameterSet &parameters, bool bMutable = false)
{
	typedef typename ActorFactory<ControllerType>::Controllers Controllers;

	static uint4 controllerParameterID = bees::GetSerializationParameters().Add("bePhysics." + utf8_string(Controllers::GetComponentType()->Name));

	const beCore::ParameterLayout &layout = bees::GetSerializationParameters();
	Controllers *pControllers = parameters.GetValueDefault< Controllers* >(layout, controllerParameterID);

	if (!pControllers)
	{
		pControllers = CreateRigidControllers<ControllerType>(parameters);
		if (bMutable)
			const_cast<beCore::ParameterSet&>(parameters).SetValue< Controllers* >(layout, controllerParameterID, pControllers);
	}

	return pControllers;
}


} // namesoace

// Constructor.
template <class RigidController>
RigidControllerSerializer<RigidController>::RigidControllerSerializer()
	: ControllerSerializer(RigidController::GetComponentType()->Name)
{
}

// Destructor.
template <class RigidController>
RigidControllerSerializer<RigidController>::~RigidControllerSerializer()
{
}

// Gets a list of creation parameters.
template <class RigidController>
beCore::ComponentParameters RigidControllerSerializer<RigidController>::GetCreationParameters() const
{
	static const beCore::ComponentParameter parameters[] = {
			beCore::ComponentParameter( utf8_ntr("Shape"), RigidShape::GetComponentType() ),
		};

	return beCore::ComponentParameters(parameters, parameters + lean::arraylen(parameters));
}

// Creates a serializable object from the given parameters.
template <class RigidController>
lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> RigidControllerSerializer<RigidController>::Create(
	const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
	typename ActorFactory<RigidController>::Controllers* rigidControllers = GetRigidControllers<RigidController>(parameters);

	RigidShape *shape = creationParameters.GetValueChecked<RigidShape*>("Shape");

	lean::scoped_ptr<RigidController> controller( rigidControllers->AddController() );
	controller->SetShape(shape);

	return controller.transfer();
}

// Loads a mesh controller from the given xml node.
template <class RigidController>
lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> RigidControllerSerializer<RigidController>::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
	typename ActorFactory<RigidController>::Controllers* rigidControllers = GetRigidControllers<RigidController>(parameters);
	
	lean::resource_ptr<RigidShape> shape;

	// Check for legacy file format
	utf8_ntr mainMaterialName = lean::get_attribute(node, "materialName");

	if (mainMaterialName.empty())
	{
		shape = physicsParameters.ResourceManager->RigidShapeCache->GetByName( lean::get_attribute(node, "shape"), true );
		// Fill new/unconfigured subsets with default material
		FillRigidShape( *shape, GetDefaultMaterial<RigidController>(*physicsParameters.ResourceManager) );
	}
	else
	{
		// Load shape & material
		AssembledShape *mainShape = physicsParameters.ResourceManager->ShapeCache->GetByFile(lean::get_attribute(node, "shape"));
		
		Material *mainMaterial = physicsParameters.ResourceManager->MaterialCache->GetByName(mainMaterialName);
		// Default material, if none specified
		if (!mainMaterial)
			mainMaterial = GetDefaultMaterial<RigidController>(*physicsParameters.ResourceManager);

		shape = ToRigidShape(*mainShape, mainMaterial, true);
		physicsParameters.ResourceManager->RigidShapeCache->SetName(
				shape,
				physicsParameters.ResourceManager->RigidShapeCache->GetUniqueName( bec::GetCachedName<utf8_ntr>(mainShape) )
			);
	}

	lean::scoped_ptr<RigidController> controller( rigidControllers->AddController() );
	controller->SetShape(shape);
	ControllerSerializer::Load(controller.get(), node, parameters, queue);

	return controller.transfer();
}

// Saves the given mesh controller to the given XML node.
template <class RigidController>
void RigidControllerSerializer<RigidController>::Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const
{
	ControllerSerializer::Save(pSerializable, node, parameters, queue);

	const RigidController &controller = static_cast<const RigidController&>(*pSerializable);
	
	if (const RigidShape *shape = controller.GetShape())
	{
		utf8_ntr name = bec::GetCachedName<utf8_ntr>(shape);
		if (!name.empty())
			lean::append_attribute( *node.document(), node, "shape", name );
		else
			LEAN_LOG_ERROR_MSG("Could not identify RigidController shape, information will be lost");

		SaveShape(shape, parameters, queue);
	}
}

const beEntitySystem::EntityControllerSerializationPlugin<RigidStaticControllerSerializer> RigidStaticControllerSerialization;
const beEntitySystem::EntityControllerSerializationPlugin<RigidDynamicControllerSerializer> RigidDynamicControllerSerialization;

} // namespace
