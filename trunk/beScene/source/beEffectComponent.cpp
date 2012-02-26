/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include <beGraphics/beEffect.h>

#include <beCore/beComponentReflector.h>
#include <beCore/beComponentTypes.h>

#include "beScene/beSerializationParameters.h"
#include "beScene/beResourceManager.h"

#include <beGraphics/beEffectCache.h>

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace beScene
{

/// Reflects effects for use in component-based editing environments.
class EffectReflector : public beCore::ComponentReflector
{
	/// Returns true, if the component can be loaded from a file.
	bool CanBeLoaded() const
	{
		return true;
	}
	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const
	{
		return utf8_ntr("fx");
	}
	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponent(const utf8_ntri &file, const beCore::ParameterSet &parameters) const
	{
		SceneParameters sceneParameters = GetSceneParameters(parameters);

		return lean::any_value<beGraphics::Effect*>(
				sceneParameters.ResourceManager->EffectCache()->GetEffect(file, nullptr, 0) // TODO: Where to get parameters from? --> create?
			);
	}

	/// Gets the component type reflected.
	utf8_ntr GetType() const
	{
		return utf8_ntr("Effect"); 
	}
};

static const beCore::ComponentTypePlugin<EffectReflector> EffectReflectorPlugin;

} // namespace
