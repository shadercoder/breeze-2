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

		lean::resource_ptr<Material> pMaterial = lean::new_resource<Material>(
				creationParameters.GetValueChecked<beGraphics::Effect*>("Effect"),
				*sceneParameters.ResourceManager->EffectCache(),
				*sceneParameters.ResourceManager->TextureCache(),
				sceneParameters.ResourceManager->MaterialCache()
			);

		sceneParameters.ResourceManager->MaterialCache()->SetMaterialName(
				creationParameters.GetValueChecked<beCore::Exchange::utf8_string>("Name"),
				pMaterial
			);

		return lean::any_value<RenderableMaterial*>(
				sceneParameters.Renderer->RenderableMaterials()->GetMaterial(pMaterial)
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

	/// Gets the name or file of the given component.
	beCore::Exchange::utf8_string GetNameOrFile(const lean::any &component, beCore::ComponentState::T *pState = nullptr) const
	{
		beCore::Exchange::utf8_string result;

		const RenderableMaterial *pMaterial = any_cast<RenderableMaterial*>(component);

		if (pMaterial)
		{
			const MaterialCache *pCache = pMaterial->GetMaterial()->GetCache();
			
			if (pCache)
			{
				bool bFile = false;
				result = pCache->GetFile(pMaterial->GetMaterial(), &bFile).to<beCore::Exchange::utf8_string>();

				if (pState)
				{
					if (bFile)
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
		return utf8_ntr("RenderableMaterial"); 
	}
};

static const beCore::ComponentTypePlugin<RenderableMaterialReflector> RenderableMaterialReflectorPlugin;

} // namespace
