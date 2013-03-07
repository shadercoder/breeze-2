/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include <beEntitySystem/beGenericGroupControllerSerializer.h>
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

} // namespace

/// Serializes mesh controllers.
template <class RigidController>
class RigidControllerSerializer : public bees::GenericGroupControllerSerializer<
	RigidController,
	typename ActorFactory<RigidController>::Controllers,
	RigidControllerSerializer<RigidController>
>
{
public:
	typedef typename ActorFactory<RigidController>::Controllers RigidControllers;

	RigidControllers* CreateControllerGroup(bees::World &world, const bec::ParameterSet &parameters) const
	{
		PhysicsParameters sceneParameters = GetPhysicsParameters(parameters);
		SceneController *sceneCtrl = LEAN_THROW_NULL(sceneParameters.SceneController);

		lean::scoped_ptr<RigidControllers> controllers = ActorFactory<RigidController>::CreateControllers(
				&world.PersistentIDs(),
				sceneParameters.Device,
				sceneParameters.Scene
			);
		controllers->SetComponentMonitor(sceneParameters.ResourceManager->Monitor());
		ActorFactory<RigidController>::LinkControllers(controllers, *sceneCtrl);

		return controllers.detach();
	}

	// Gets a list of creation parameters.
	beCore::ComponentParameters GetCreationParameters() const
	{
		static const beCore::ComponentParameter parameters[] = {
				beCore::ComponentParameter( utf8_ntr("Shape"), RigidShape::GetComponentType() ),
			};

		return beCore::ComponentParameters(parameters, parameters + lean::arraylen(parameters));
	}

	// Creates a serializable object from the given parameters.
	lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> Create(
		const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
		RigidControllers* rigidControllers = GetControllerGroup(parameters);

		RigidShape *shape = creationParameters.GetValueChecked<RigidShape*>("Shape");

		lean::scoped_ptr<RigidController> controller( rigidControllers->AddController() );
		controller->SetShape(shape);

		return controller.transfer();
	}

	// Loads a mesh controller from the given xml node.
	lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const LEAN_OVERRIDE
	{
		beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
		RigidControllers* rigidControllers = GetControllerGroup(parameters);
	
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
	void Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const LEAN_OVERRIDE
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
};

const beEntitySystem::EntityControllerSerializationPlugin< RigidControllerSerializer<RigidStaticController> > RigidStaticControllerSerialization;
const beEntitySystem::EntityControllerSerializationPlugin< RigidControllerSerializer<RigidDynamicController> > RigidDynamicControllerSerialization;

} // namespace