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

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

/// Reflects shapes for use in component-based editing environments.
class ShapeReflector : public beCore::ComponentReflector
{
	/// Returns true, if the component can be loaded from a file.
	bool CanBeLoaded() const
	{
		return true;
	}
	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const
	{
		return utf8_ntr("shape");
	}
	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponent(const utf8_ntri &file, const beCore::ParameterSet &parameters) const
	{
		PhysicsParameters physicsParameters = GetPhysicsParameters(parameters);

		return lean::any_value<ShapeCompound*>(
				physicsParameters.ResourceManager->ShapeCache()->GetByFile(file)
			);
	}

	/// Gets the name or file of the given component.
	beCore::Exchange::utf8_string GetNameOrFile(const lean::any &component, beCore::ComponentState::T *pState = nullptr) const
	{
		beCore::Exchange::utf8_string result;

		const ShapeCompound *shape = any_cast<ShapeCompound*>(component);

		if (shape)
		{
			const ShapeCache *cache = shape->GetCache();
			
			if (cache)
			{
				result = cache->GetFile(shape).to<beCore::Exchange::utf8_string>();
				bool bFiled = !result.empty();

				if (!bFiled)
					result = cache->GetName(shape).to<beCore::Exchange::utf8_string>();

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
		return utf8_ntr("PhysicsShape"); 
	}
};

static const beCore::ComponentTypePlugin<ShapeReflector> ShapeReflectorPlugin;

} // namespace
