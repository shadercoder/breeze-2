/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beMaterial.h"

#include <beCore/beComponentReflector.h>
#include <beCore/beComponentTypes.h>

#include "bePhysics/beSerializationParameters.h"
#include "bePhysics/beResourceManager.h"

#include "bePhysics/beMaterialCache.h"

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

extern const beCore::ComponentType MaterialType = { "PhysicsMaterial" };

/// Gets the component type.
const beCore::ComponentType* Material::GetComponentType()
{
	return &MaterialType;
}

/// Gets the component type.
const beCore::ComponentType* Material::GetType() const
{
	return &MaterialType;
}

/// Reflects materials for use in component-based editing environments.
class MaterialReflector : public beCore::ComponentReflector
{
	/// Gets principal component flags.
	uint4 GetComponentFlags() const LEAN_OVERRIDE
	{
		return bec::ComponentFlags::Creatable | bec::ComponentFlags::Cloneable; // bec::ComponentFlags::Filed | 
	}
	/// Gets specific component flags.
	uint4 GetComponentFlags(const lean::any &component) const LEAN_OVERRIDE
	{
		uint4 flags = bec::ComponentFlags::NameMutable; // | bec::ComponentFlags::FileMutable

/*		if (const Material *material = any_cast_default<Material*>(component))
			if (const MaterialCache *cache = material->GetCache())
				if (!cache->GetFile(material).empty())
					flags |= bec::ComponentState::Filed;
*/
		return flags;
	}

	/// Gets information on the components currently available.
	bec::ComponentInfoVector GetComponentInfo(const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return GetPhysicsParameters(parameters).ResourceManager->MaterialCache()->GetInfo();
	}
	
	/// Gets the component info.
	bec::ComponentInfo GetInfo(const lean::any &component) const LEAN_OVERRIDE
	{
		bec::ComponentInfo result;

		if (const Material *material = any_cast_default<Material*>(component))
			if (const MaterialCache *cache = material->GetCache())
				result = cache->GetInfo(material);

		return result;
	}
	
	/// Creates a component from the given parameters.
	lean::cloneable_obj<lean::any, true> CreateComponent(const utf8_ntri &name, const beCore::Parameters &creationParameters,
		const beCore::ParameterSet &parameters, const lean::any *pPrototype, const lean::any *pReplace) const LEAN_OVERRIDE
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);

		lean::resource_ptr<Material> material = CreateMaterial(*physicsParameters.Device, 1.0f, 1.0f, 0.0f);
		
		if (Material *pPrototypeMaterial = lean::any_cast_default<Material*>(pPrototype))
			TransferProperties(*material, *pPrototypeMaterial);

		if (Material *pToBeReplaced = lean::any_cast_default<Material*>(pReplace))
			physicsParameters.ResourceManager->MaterialCache->Replace(pToBeReplaced, material);
		else
			physicsParameters.ResourceManager->MaterialCache->SetName(material, name);

		return bec::any_resource_t<Material>::t(material);
	}

	/// Sets the component name.
	void SetName(const lean::any &component, const utf8_ntri &name) const LEAN_OVERRIDE
	{
		if (Material *material = any_cast_default<Material*>(component))
			if (MaterialCache *cache = material->GetCache())
			{
				cache->SetName(material, name);
				return;
			}

		LEAN_THROW_ERROR_CTX("Unknown material cannot be renamed", name.c_str());
	}

	/// Gets a component by name.
	lean::cloneable_obj<lean::any, true> GetComponentByName(const utf8_ntri &name, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		return bec::any_resource_t<Material>::t(
				GetPhysicsParameters(parameters).ResourceManager->MaterialCache()->GetByName(name)
			);
	}

	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const LEAN_OVERRIDE
	{
		return utf8_ntr("material.physics.xml");
	}
/*	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponentByFile(const utf8_ntri &file,
		const beCore::Parameters &fileParameters, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);

		return bec::any_resource_t<Material>::t(
				physicsParameters.ResourceManager->MaterialCache()->GetByFile(file)
			);
	}
*/
	/// Gets the component type reflected.
	const beCore::ComponentType* GetType() const LEAN_OVERRIDE
	{
		return &MaterialType; 
	}
};

static const beCore::ComponentReflectorPlugin<MaterialReflector> MaterialReflectorPlugin(&MaterialType);

} // namespace
