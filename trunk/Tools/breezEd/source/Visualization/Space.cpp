#include "stdafx.h"
#include "Visualization/Space.h"

#include <beGraphics/beEffectCache.h>
#include <beGraphics/beMaterialCache.h>
#include <beGraphics/beMaterialConfig.h>
#include <beScene/beMeshCache.h>
#include <beScene/beRenderableMaterialCache.h>

namespace
{

/// Creates widget data for the given widget.
SpaceVisualizer::WidgetData createWidgetData(const lean::utf8_ntri &meshFile, const utf8_ntri &effectFile, const utf8_ntri &effectFlags,
											 besc::ResourceManager &resources, besc::EffectDrivenRenderer &renderer)
{
	SpaceVisualizer::WidgetData result;

	// Create Material
	lean::resource_ptr<beGraphics::Effect> effect = resources.EffectCache->GetByFile(effectFile, effectFlags, "");
	lean::resource_ptr<beGraphics::Material> material = beGraphics::CreateMaterial(&effect.get(), 1, *resources.EffectCache());
	lean::resource_ptr<beGraphics::MaterialConfig> materialConfig = beGraphics::CreateMaterialConfig();
	material->SetConfigurations(&materialConfig.get(), 1);
	result.material = material;

	// Setup up configuration prototype
	{
		lean::com_ptr<beGraphics::MaterialReflectionBinding> binding = material->GetConfigBinding(0);
		
		// Transfer color to configuration
		uint4 colorPropertyBindingID = binding->GetPropertyID("Color");
		bem::fvec4 color;
		binding->GetProperty(colorPropertyBindingID, color.data(), 4);
		binding->SetProperty(colorPropertyBindingID, color.data(), 4);
	}
	result.colorID = materialConfig->GetPropertyID("Color");

	result.mesh = resources.MeshCache()->GetByFile(meshFile);

	return result;
}

/// Creates a new widget from the given widget data.
SpaceVisualizer::Widget createWidget(const SpaceVisualizer::WidgetData &data, bees::Entities &entities, besc::MeshControllers &meshes,
									 besc::RenderableMaterialCache &renderableMaterials)
{
	SpaceVisualizer::Widget result;

	// Clone material & configuration
	lean::resource_ptr<beg::Material> material = beg::CreateMaterial( *data.material );
	material->SetConfiguration( 0, beg::CreateMaterialConfig( *data.material->GetConfiguration(0) ).get() );
	
	// Create mesh
	lean::resource_ptr<besc::RenderableMesh> mesh = beScene::ToRenderableMesh(*data.mesh, renderableMaterials.GetMaterial(material), true);
	
	// Create entity
	lean::scoped_ptr<beEntitySystem::Entity> entity( entities.AddEntity("Widget", bees::Entities::AnonymousPersistentID) );
	{
		lean::scoped_ptr<beScene::MeshController> meshController( meshes.AddController() );
		meshController->SetMesh(mesh);
		entity->AddController(meshController.move_ptr());
	}
	
	result.entity = entity.detach();
	return result;
}

} // namespace

// Constructor.
SpaceVisualizer::SpaceVisualizer(beScene::ResourceManager &resources, beScene::EffectDrivenRenderer &renderer)
	: m_world( new_resource bees::World("WidgetWorld") ),
	m_simulation( new_resource bees::Simulation("WidgetSimulation") )
{
	m_data[WidgetType::Arrow] = createWidgetData("Static/UI/TranslateWidget.mesh", "Materials/Widget.fx", "", resources, renderer);
	m_data[WidgetType::Circle] = createWidgetData("Static/UI/RotateWidget.mesh", "Materials/Widget.fx", "DARKEN_SPHERE_BACK", resources, renderer);
	m_data[WidgetType::Plug] = createWidgetData("Static/UI/ScaleWidget.mesh", "Materials/Widget.fx", "", resources, renderer);

	m_scene = new besc::RenderingController(renderer.Pipeline, nullptr);
	m_world->Controllers().AddControllerKeep(m_scene);

	m_meshes = besc::CreateMeshControllers(&m_world->PersistentIDs()).detach();
	m_world->Controllers().AddControllerKeep(m_meshes);

	Attach(m_world, m_simulation);
}

// Destructor.
SpaceVisualizer::~SpaceVisualizer()
{
}
