/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beShapes.h"

#include <beCore/beComponentReflector.h>
#include <beCore/beComponentTypes.h>

#include "bePhysics/beSerializationParameters.h"
#include "bePhysics/beResourceManager.h"

#include "bePhysics/beShapeCache.h"
#include "bePhysics/beRigidShapeCache.h"

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

extern const beCore::ComponentType RigidShapeType;

/// Reflects shapes for use in component-based editing environments.
class RigidShapeReflector : public beCore::ComponentReflector
{
	/// Gets principal component flags.
	uint4 GetComponentFlags() const LEAN_OVERRIDE
	{
		return bec::ComponentFlags::Creatable | bec::ComponentFlags::Cloneable;
	}
	/// Gets specific component flags.
	uint4 GetComponentFlags(const lean::any &component) const LEAN_OVERRIDE
	{
		uint4 flags = bec::ComponentFlags::NameMutable; // | bec::ComponentFlags::FileMutable

/*		if (const RigidShape *shape = any_cast_default<RigidShape*>(component))
			if (const RigidShapeCache *cache = shape->GetCache())
				if (!cache->GetFile(shape).empty())
					flags |= bec::ComponentState::Filed;
*/
		return flags;
	}

	/// Gets information on the components currently available.
	bec::ComponentInfoVector GetComponentInfo(const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return GetPhysicsParameters(parameters).ResourceManager->RigidShapeCache->GetInfo();
	}
	
	/// Gets the component info.
	bec::ComponentInfo GetInfo(const lean::any &component) const LEAN_OVERRIDE
	{
		bec::ComponentInfo result;

		if (const RigidShape *shape = any_cast_default<RigidShape*>(component))
			if (const RigidShapeCache *cache = shape->GetCache())
				result = cache->GetInfo(shape);

		return result;
	}
	
	/// Gets a list of creation parameters.
	beCore::ComponentParameters GetCreationParameters() const LEAN_OVERRIDE
	{
		static const beCore::ComponentParameter parameters[] = {
				beCore::ComponentParameter(utf8_ntr("Source"), AssembledShape::GetComponentType(), bec::ComponentParameterFlags::Deducible),
				beCore::ComponentParameter(utf8_ntr("Material"), Material::GetComponentType(), bec::ComponentParameterFlags::Optional)
			};

		return beCore::ComponentParameters(parameters, parameters + lean::arraylen(parameters));
	}
	/// Creates a component from the given parameters.
	lean::cloneable_obj<lean::any, true> CreateComponent(const utf8_ntri &name, const beCore::Parameters &creationParameters,
		const beCore::ParameterSet &parameters, const lean::any *pPrototype, const lean::any *pReplace) const LEAN_OVERRIDE
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);
		RigidShape *pPrototypeShape = lean::any_cast_default<RigidShape*>(pPrototype);

		const AssembledShape *sourceShape = creationParameters.GetValueDefault<AssembledShape*>("Source");
		if (!sourceShape && pPrototypeShape)
			sourceShape = pPrototypeShape->GetSource();

		lean::resource_ptr<RigidShape> shape = ToRigidShape(*LEAN_THROW_NULL(sourceShape), nullptr, true);

		if (pPrototypeShape)
			TransferMaterials(*pPrototypeShape, *shape);

		if (Material *fillMaterial = creationParameters.GetValueDefault<Material*>("Material"))
			FillRigidShape(*shape, fillMaterial);

		if (RigidShape *pToBeReplaced = lean::any_cast_default<RigidShape*>(pReplace))
			physicsParameters.ResourceManager->RigidShapeCache->Replace(pToBeReplaced, shape);
		else
			physicsParameters.ResourceManager->RigidShapeCache->SetName(shape, name);

		return bec::any_resource_t<RigidShape>::t(shape);
	}
	
	// Gets a list of creation parameters.
	void GetCreationInfo(const lean::any &component, bec::Parameters &creationParameters, bec::ComponentInfo *pInfo = nullptr) const LEAN_OVERRIDE
	{
		if (const RigidShape *shape = any_cast_default<RigidShape*>(component))
			if (const RigidShapeCache *cache = shape->GetCache())
			{
				if (pInfo)
					*pInfo = cache->GetInfo(shape);

				creationParameters.SetValue<const AssembledShape*>("Source", shape->GetSource());
			}
	}

	/// Sets the component name.
	void SetName(const lean::any &component, const utf8_ntri &name) const LEAN_OVERRIDE
	{
		if (RigidShape *shape = any_cast_default<RigidShape*>(component))
			if (RigidShapeCache *cache = shape->GetCache())
			{
				cache->SetName(shape, name);
				return;
			}

		LEAN_THROW_ERROR_CTX("Unknown shape cannot be renamed", name.c_str());
	}

	/// Gets a component by name.
	lean::cloneable_obj<lean::any, true> GetComponentByName(const utf8_ntri &name, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return bec::any_resource_t<RigidShape>::t(
				GetPhysicsParameters(parameters).ResourceManager->RigidShapeCache()->GetByName(name)
			);
	}

	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const LEAN_OVERRIDE
	{
		return utf8_ntr("rigidshape");
	}
/*	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponentByFile(const utf8_ntri &file,
		const beCore::Parameters &fileParameters, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return bec::any_resource_t<RigidShape>::t(
				GetPhysicsParameters(parameters).ResourceManager->RigidShapeCache()->GetByFile(file)
			);
	}
*/
	/// Gets the component type reflected.
	const beCore::ComponentType* GetType() const LEAN_OVERRIDE
	{
		return &RigidShapeType; 
	}
};

static const beCore::ComponentReflectorPlugin<RigidShapeReflector> RigidShapeReflectorPlugin(&RigidShapeType);

extern const beCore::ComponentType AssembledShapeType;

/// Reflects shapes for use in component-based editing environments.
class ShapeReflector : public beCore::ComponentReflector
{
	/// Gets principal component flags.
	uint4 GetComponentFlags() const LEAN_OVERRIDE
	{
		return bec::ComponentFlags::Filed;
	}
	/// Gets specific component flags.
	uint4 GetComponentFlags(const lean::any &component) const LEAN_OVERRIDE
	{
		uint4 flags = bec::ComponentFlags::NameMutable; // | bec::ComponentFlags::FileMutable

		if (const AssembledShape *shape = any_cast_default<AssembledShape*>(component))
			if (const ShapeCache *cache = shape->GetCache())
				if (!cache->GetFile(shape).empty())
					flags |= bec::ComponentState::Filed;

		return flags;
	}

	/// Gets information on the components currently available.
	bec::ComponentInfoVector GetComponentInfo(const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return GetPhysicsParameters(parameters).ResourceManager->ShapeCache()->GetInfo();
	}
	
	/// Gets the component info.
	bec::ComponentInfo GetInfo(const lean::any &component) const LEAN_OVERRIDE
	{
		bec::ComponentInfo result;

		if (const AssembledShape *shape = any_cast_default<AssembledShape*>(component))
			if (const ShapeCache *cache = shape->GetCache())
				result = cache->GetInfo(shape);

		return result;
	}
	
	/// Sets the component name.
	void SetName(const lean::any &component, const utf8_ntri &name) const LEAN_OVERRIDE
	{
		if (AssembledShape *shape = any_cast_default<AssembledShape*>(component))
			if (ShapeCache *cache = shape->GetCache())
			{
				cache->SetName(shape, name);
				return;
			}

		LEAN_THROW_ERROR_CTX("Unknown shape cannot be renamed", name.c_str());
	}

	/// Gets a component by name.
	lean::cloneable_obj<lean::any, true> GetComponentByName(const utf8_ntri &name, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return bec::any_resource_t<AssembledShape>::t(
				GetPhysicsParameters(parameters).ResourceManager->ShapeCache()->GetByName(name)
			);
	}

	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const LEAN_OVERRIDE
	{
		return utf8_ntr("shape");
	}
	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponentByFile(const utf8_ntri &file,
		const beCore::Parameters &fileParameters, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return bec::any_resource_t<AssembledShape>::t(
				GetPhysicsParameters(parameters).ResourceManager->ShapeCache()->GetByFile(file)
			);
	}

	/// Gets the component type reflected.
	const beCore::ComponentType* GetType() const LEAN_OVERRIDE
	{
		return &AssembledShapeType; 
	}
};

static const beCore::ComponentReflectorPlugin<ShapeReflector> ShapeReflectorPlugin(&AssembledShapeType);

} // namespace
