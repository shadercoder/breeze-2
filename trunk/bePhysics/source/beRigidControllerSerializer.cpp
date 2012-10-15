/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidControllerSerializer.h"
#include "bePhysics/beRigidStaticController.h"
#include "bePhysics/beRigidDynamicController.h"

#include <bePhysics/beRigidActors.h>

#include <beEntitySystem/beControllerSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include "bePhysics/beSerializationParameters.h"
#include <beCore/beParameterSet.h>
#include <beCore/beExchangeContainers.h>

#include "bePhysics/beInlineMaterialSerialization.h"

#include "bePhysics/beResourceManager.h"
#include "bePhysics/beMaterialCache.h"
#include "bePhysics/beShapeCache.h"

#include "bePhysics/beMaterial.h"
#include "bePhysics/beShapes.h"

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
	typedef RigidStatic Actor;

	static lean::resource_ptr<RigidStatic, true> CreateActor(Device &device, const ShapeCompound &shape)
	{
		return CreateStaticFromShape(device, shape);
	}
};

template <>
struct ActorFactory<RigidDynamicController>
{
	typedef RigidStatic Actor;

	static lean::resource_ptr<RigidDynamic, true> CreateActor(Device &device, const ShapeCompound &shape)
	{
		return CreateDynamicFromShape(device, shape, -100.0f);
	}
};

} // namesoace

// Constructor.
template <class RigidController>
RigidControllerSerializer<RigidController>::RigidControllerSerializer()
	: ControllerSerializer(RigidController::GetControllerType())
{
}

// Destructor.
template <class RigidController>
RigidControllerSerializer<RigidController>::~RigidControllerSerializer()
{
}

// Gets a list of creation parameters.
template <class RigidController>
typename RigidControllerSerializer<RigidController>::SerializationParameters RigidControllerSerializer<RigidController>::GetCreationParameters() const
{
	static const beEntitySystem::CreationParameter parameters[] = {
			beEntitySystem::CreationParameter( utf8_ntr("Shape"), utf8_ntr("PhysicsShape") ),
			beEntitySystem::CreationParameter( utf8_ntr("Material"), utf8_ntr("PhysicsMaterial"), true )
		};

	return SerializationParameters(parameters, parameters + lean::arraylen(parameters));
}

// Creates a serializable object from the given parameters.
template <class RigidController>
lean::resource_ptr<beEntitySystem::Controller, true> RigidControllerSerializer<RigidController>::Create(
	const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);

	// Get mesh
	const ShapeCompound *shape = creationParameters.GetValueChecked<const ShapeCompound*>("Shape");

	// (Optionally) get material
	const Material *pMaterial = creationParameters.GetValueDefault<const Material*>("Material");

	// Default material, if none specified
	if (!pMaterial)
		pMaterial = GetDefaultMaterial<RigidController>(*physicsParameters.ResourceManager);
	
	lean::resource_ptr<RigidController> pController = lean::new_resource<RigidController>(
			entityParameters.Entity, physicsParameters.SceneController,
			ActorFactory<RigidController>::CreateActor(*physicsParameters.Device, *shape).get(),
			shape, pMaterial
		);

	return pController.transfer();
}

namespace
{

/// Gets a shape from the given cache.
inline ShapeCompound* GetShape(const rapidxml::xml_node<lean::utf8_t> &node, ShapeCache &cache)
{
	utf8_ntr file = lean::get_attribute(node, "shape");

	if (!file.empty())
		return cache.GetByFile(file);
	else
	{
		utf8_ntr name = lean::get_attribute(node, "shapeName");

		if (!name.empty())
			return cache.GetByName(name, true);
	}

	return nullptr;
}

/// Gets a material from the given cache.
inline Material* GetMaterial(const rapidxml::xml_node<lean::utf8_t> &node, MaterialCache &cache)
{
	utf8_ntr file = lean::get_attribute(node, "material");

	if (!file.empty())
		return cache.GetByFile(file);
	else
	{
		utf8_ntr name = lean::get_attribute(node, "materialName");

		if (!name.empty())
			return cache.GetByName(name, true);
	}

	return nullptr;
}

} // namespace

// Loads a mesh controller from the given xml node.
template <class RigidController>
lean::resource_ptr<beEntitySystem::Controller, true> RigidControllerSerializer<RigidController>::Load(const rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::LoadJob> &queue) const
{
	beEntitySystem::EntitySystemParameters entityParameters = beEntitySystem::GetEntitySystemParameters(parameters);
	PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
	
	// Load shape & material
	ShapeCompound *pMainShape = GetShape(node, *physicsParameters.ResourceManager->ShapeCache());
	Material *pMainMaterial = GetMaterial(node, *physicsParameters.ResourceManager->MaterialCache());

	// Shape required
	if (!pMainShape)
		LEAN_THROW_ERROR_MSG("RigidController does not specify a shape");

	// Default material, if none specified
	if (!pMainMaterial)
		pMainMaterial = GetDefaultMaterial<RigidController>(*physicsParameters.ResourceManager);

	lean::resource_ptr<RigidController> pController = lean::new_resource<RigidController>(
			entityParameters.Entity, physicsParameters.SceneController,
			ActorFactory<RigidController>::CreateActor(*physicsParameters.Device, *pMainShape).get(),
			pMainShape, pMainMaterial
		);

	ControllerSerializer::Load(pController, node, parameters, queue);

	return pController.transfer();
}

namespace
{

/// Gets the file, if available.
template <class Resource, class ResourceCache>
LEAN_INLINE beCore::Exchange::utf8_string MaybeGetFile(const Resource *pResource, const ResourceCache *pCache, bool &bIsFile)
{
	beCore::Exchange::utf8_string result;
	
	if (pCache)
	{
		utf8_ntr fileOrName = pCache->GetFile(pResource);
		bIsFile = !fileOrName.empty();

		if (bIsFile)
			result = pCache->GetPathResolver().Shorten(fileOrName);
		else
		{
			fileOrName = pCache->GetName(pResource);
			result.assign(fileOrName.begin(), fileOrName.end());
		}
	}

	return result;
}
/// Gets the file, if available.
template <class Resource>
LEAN_INLINE beCore::Exchange::utf8_string MaybeGetFile(const Resource *pResource, bool &bIsFile)
{
	return MaybeGetFile(pResource, (pResource) ? pResource->GetCache() : nullptr, bIsFile);
}

// Saves the given resource in one of the given attributes. Returns true, if saved by name, false otherwise.
template <class Resource>
inline bool AppendResourceAttribute(rapidxml::xml_node<lean::utf8_t> &node, const utf8_ntri &fileAtt, const utf8_ntri &nameAtt, const Resource *pResource)
{
	bool bFile = false;
	beCore::Exchange::utf8_string file = MaybeGetFile(pResource, bFile);

	if (bFile)
	{
		lean::append_attribute(*node.document(), node, fileAtt, file);
		return false;
	}
	else if (!file.empty())
	{
		lean::append_attribute(*node.document(), node, nameAtt, file);
		return true;
	}
	else
		// TODO: Also print persistent ID
		LEAN_LOG_ERROR("Could not identify RigidController #TODO resource '" << fileAtt.c_str() << "', will be lost");

	return false;
}

} // namespace

// Saves the given mesh controller to the given XML node.
template <class RigidController>
void RigidControllerSerializer<RigidController>::Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::SaveJob> &queue) const
{
	ControllerSerializer::Save(pSerializable, node, parameters, queue);

	const RigidController &controller = static_cast<const RigidController&>(*pSerializable);
	
	const ShapeCompound *pMainShape = controller.GetShape();
	const Material *pMainMaterial = controller.GetMaterial();
	
	AppendResourceAttribute(node, "shape", "shapeName", pMainShape);
	if (AppendResourceAttribute(node, "material", "materialName", pMainMaterial))
		SaveMaterial(pMainMaterial, parameters, queue);
}

const beEntitySystem::ControllerSerializationPlugin<RigidStaticControllerSerializer> RigidStaticControllerSerialization;
const beEntitySystem::ControllerSerializationPlugin<RigidDynamicControllerSerializer> RigidDynamicControllerSerialization;

} // namespace
