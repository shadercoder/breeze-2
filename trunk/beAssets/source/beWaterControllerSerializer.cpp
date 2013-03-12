/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beAssetsInternal/stdafx.h"
#include <beEntitySystem/beGenericGroupControllerSerializer.h>
#include "beAssets/beWaterControllers.h"
#include <beScene/beRenderingController.h>

#include <beEntitySystem/beWorld.h>
#include <beEntitySystem/beSerialization.h>

#include <beEntitySystem/beSerializationParameters.h>
#include <beScene/beSerializationParameters.h>

#include <beCore/beParameterSet.h>
#include <beCore/beParameters.h>
#include <beCore/beExchangeContainers.h>

#include "beScene/beRenderableMaterial.h"
#include "beScene/beRenderableMaterialCache.h"
#include "beScene/beInlineMaterialSerialization.h"

#include "beScene/beResourceManager.h"
#include "beScene/beEffectDrivenRenderer.h"

#include <lean/xml/numeric.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beAssets
{

/// Serializes water controllers.
class WaterControllerSerializer : public bees::GenericGroupControllerSerializer<WaterController, WaterControllers, WaterControllerSerializer>
{
public:
	WaterControllers* CreateControllerGroup(bees::World &world, const bec::ParameterSet &parameters) const
	{
		besc::SceneParameters sceneParameters = besc::GetSceneParameters(parameters);
		besc::RenderingController *renderingCtrl = LEAN_THROW_NULL(sceneParameters.RenderingController);

		lean::scoped_ptr<WaterControllers> controllers = CreateWaterControllers(
				&world.PersistentIDs(),
				sceneParameters.Renderer->PerspectivePool(), *sceneParameters.Renderer->Pipeline(),
				*sceneParameters.Renderer->Device()
			);
		controllers->SetComponentMonitor(sceneParameters.ResourceManager->Monitor);
		renderingCtrl->AddRenderable(controllers);

		return controllers.detach();
	}
	
	// Gets a list of creation parameters.
	beCore::ComponentParameters GetCreationParameters() const LEAN_OVERRIDE
	{
		static const beCore::ComponentParameter parameters[] = {
				beCore::ComponentParameter( utf8_ntr("Material"), besc::RenderableMaterial::GetComponentType() )
			};
		return beCore::ComponentParameters(parameters, parameters + lean::arraylen(parameters));
	}

	// Creates a serializable object from the given parameters.
	lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> Create(
		const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const LEAN_OVERRIDE
	{
		WaterControllers* waterControllers = GetControllerGroup(parameters);

		// Get parameters
		besc::RenderableMaterial *material = creationParameters.GetValueChecked<besc::RenderableMaterial*>("Material");

		// Create controller
		lean::scoped_ptr<WaterController> controller( waterControllers->AddController() );
		controller->SetMaterial(material);

		return controller.transfer();
	}

	// Loads a mesh controller from the given xml node.
	lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const LEAN_OVERRIDE
	{
		besc::SceneParameters sceneParameters = besc::GetSceneParameters(parameters);
		WaterControllers* waterControllers = GetControllerGroup(parameters);

		lean::resource_ptr<besc::RenderableMaterial> material = sceneParameters.Renderer->RenderableMaterials->GetMaterial(
				sceneParameters.ResourceManager->MaterialCache->GetByName(lean::get_attribute(node, "material"), true)
			);
		
		lean::scoped_ptr<WaterController> controller( waterControllers->AddController() );
		controller->SetMaterial(material);
		ControllerSerializer::Load(controller, node, parameters, queue);

		return controller.transfer();
	}

	// Saves the given mesh controller to the given XML node.
	void Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const LEAN_OVERRIDE
	{
		ControllerSerializer::Save(pSerializable, node, parameters, queue);

		const WaterController &waterController = static_cast<const WaterController&>(*pSerializable);
	
		if (const besc::RenderableMaterial *material = waterController.GetMaterial())
		{
			utf8_ntr name = bec::GetCachedName<utf8_ntr>(material->GetMaterial());
			if (!name.empty())
				lean::append_attribute( *node.document(), node, "material", name );
			else
				LEAN_LOG_ERROR_MSG("Could not identify WaterController material, information will be lost");

			besc::SaveMaterial(material->GetMaterial(), parameters, queue);
		}
	}
};

const beEntitySystem::EntityControllerSerializationPlugin<WaterControllerSerializer> WaterControllerSerialization;

} // namespace
