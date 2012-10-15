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

/// Reflects materials for use in component-based editing environments.
class MaterialReflector : public beCore::ComponentReflector
{
	/// Returns true, if the component can be loaded from a file.
	bool CanBeLoaded() const
	{
		return true;
	}
	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const
	{
		return utf8_ntr("material.physics.xml");
	}
	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponent(const utf8_ntri &file, const beCore::ParameterSet &parameters) const
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);

		return lean::any_value<Material*>(
				physicsParameters.ResourceManager->MaterialCache()->GetByFile(file)
			);
	}

	/// Gets the name or file of the given component.
	beCore::Exchange::utf8_string GetNameOrFile(const lean::any &component, beCore::ComponentState::T *pState = nullptr) const
	{
		beCore::Exchange::utf8_string result;

		const Material *material = any_cast<Material*>(component);

		if (material)
		{
			const MaterialCache *cache = material->GetCache();
			
			if (cache)
			{
				result = cache->GetFile(material).to<beCore::Exchange::utf8_string>();
				bool bFiled = !result.empty();

				if (!bFiled)
					result = cache->GetName(material).to<beCore::Exchange::utf8_string>();

				if (pState)
				{
					if (bFiled)
						*pState = beCore::ComponentState::Filed;
					else if (!result.empty())
						*pState = beCore::ComponentState::Named;
					else
						*pState = beCore::ComponentState::Unknown;
				}
			}
			else if (pState)
				*pState = beCore::ComponentState::Unknown;
		}
		else if (pState)
			*pState = beCore::ComponentState::NotSet;

		return result;
	}

	/// Gets the component type reflected.
	utf8_ntr GetType() const
	{
		return utf8_ntr("PhysicsMaterial"); 
	}
};

static const beCore::ComponentTypePlugin<MaterialReflector> MaterialReflectorPlugin;

} // namespace
