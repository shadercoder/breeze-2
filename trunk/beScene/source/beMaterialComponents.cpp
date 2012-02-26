/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMaterial.h"
#include "beScene/beRenderableMaterial.h"

#include <beCore/beComponentReflector.h>
#include <beCore/beComponentTypes.h>

#include "beScene/beSerializationParameters.h"
#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"

#include "beScene/beMaterialCache.h"
#include "beScene/beRenderableMaterialCache.h"

#include <beGraphics/beEffectCache.h>

#include <lean/logging/log.h>
#include <lean/logging/errors.h>

namespace beScene
{

/// Reflects materials for use in component-based editing environments.
class RenderableMaterialReflector : public beCore::ComponentReflector
{
	/// Returns true, if the component can be created.
	bool CanBeCreated() const
	{
		return true;
	}
	/// Gets a list of creation parameters.
	beCore::ComponentParameters GetCreationParameters() const
	{
		static const beCore::ComponentParameter parameters[] = {
				beCore::ComponentParameter(utf8_ntr("Effect"), utf8_ntr("Effect")),
				beCore::ComponentParameter(utf8_ntr("Name"), utf8_ntr("String"))
			};

		return beCore::ComponentParameters(parameters, parameters + lean::arraylen(parameters));
	}
	/// Creates a component from the given parameters.
	lean::cloneable_obj<lean::any, true> CreateComponent(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
	{
		SceneParameters sceneParameters = GetSceneParameters(parameters);

		return lean::any_value<RenderableMaterial*>(
				sceneParameters.Renderer->RenderableMaterials()->GetMaterial(
					sceneParameters.ResourceManager->MaterialCache()->GetMaterial(
						creationParameters.GetValueChecked<beGraphics::Effect*>("Effect"), // TODO: Catch nullptr
						creationParameters.GetValueChecked<beCore::Exchange::utf8_string>("Name")
					)
				)
			);
	}

	/// Returns true, if the component can be named.
	bool HasName() const
	{
		return true;
	}
	/// Returns true, if the component can be named.
	bool CanBeNamed() const
	{
		return true;
	}
	/// Gets a component by name.
	lean::cloneable_obj<lean::any, true> GetComponentByName(const utf8_ntri &name, const beCore::ParameterSet &parameters) const
	{
		SceneParameters sceneParameters = GetSceneParameters(parameters);

		return lean::any_value<RenderableMaterial*>(
				sceneParameters.Renderer->RenderableMaterials()->GetMaterial(
					sceneParameters.ResourceManager->MaterialCache()->GetMaterialByName(name)
				)
			);
	}

	/// Returns true, if the component can be loaded from a file.
	bool CanBeLoaded() const
	{
		return true;
	}
	/// Gets a fitting file extension, if available.
	utf8_ntr GetFileExtension() const
	{
		return utf8_ntr("material.xml");
	}
	/// Gets a component from the given file.
	lean::cloneable_obj<lean::any, true> GetComponent(const utf8_ntri &file, const beCore::ParameterSet &parameters) const
	{
		SceneParameters sceneParameters = GetSceneParameters(parameters);

		return lean::any_value<RenderableMaterial*>(
				sceneParameters.Renderer->RenderableMaterials()->GetMaterial(
					sceneParameters.ResourceManager->MaterialCache()->GetMaterial(file)
				)
			);
	}

	/// Gets the component type reflected.
	utf8_ntr GetType() const
	{
		return utf8_ntr("RenderableMaterial"); 
	}
};

static const beCore::ComponentTypePlugin<RenderableMaterialReflector> RenderableMaterialReflectorPlugin;

} // namespace
