#ifndef WIDGETS_H
#define WIDGETS_H

#include "breezEd.h"

#include <beEntitySystem/beEntities.h>
#include <beScene/beMeshControllers.h>

#include <beScene/beResourceManager.h>
#include <beScene/beMeshCache.h>
#include <beGraphics/beEffectCache.h>
#include <beGraphics/beMaterial.h>
#include <beGraphics/beMaterialConfig.h>
#include <beScene/beRenderableMaterialCache.h>
#include <beScene/beRenderableMesh.h>

#include <beScene/beEffectDrivenRenderer.h>

#include <lean/smart/resource_ptr.h>

/// Creates a transform widget entity of the given color and mesh.
inline lean::scoped_ptr<beEntitySystem::Entity> createWidgetMesh(const lean::utf8_ntri &mesh, const beMath::fvec4 &color, const utf8_ntri &flags,
	bees::Entities &entities, beScene::MeshControllers &meshes, beScene::ResourceManager &resources, beScene::EffectDrivenRenderer &renderer)
{
	// TODO: NO HARD-CODED PATHS
	lean::resource_ptr<beGraphics::Effect> effect = resources.EffectCache()->GetByFile("Materials/Widget.fx", flags, "");
	lean::resource_ptr<beGraphics::Material> material = beGraphics::CreateMaterial(&effect.get(), 1, *resources.EffectCache());
	lean::resource_ptr<beGraphics::MaterialConfig> materialConfig = beGraphics::CreateMaterialConfig();
	material->SetConfigurations(&materialConfig.get(), 1);
	
	lean::com_ptr<beGraphics::MaterialReflectionBinding> binding = material->GetConfigBinding(0);
	binding->SetProperty( binding->GetPropertyID("Color"), color.data(), 4 );

	lean::resource_ptr<beScene::RenderableMesh> renderableMesh = beScene::ToRenderableMesh(
		*resources.MeshCache()->GetByFile(mesh),
		renderer.RenderableMaterials()->GetMaterial(material), true );

	lean::scoped_ptr<beEntitySystem::Entity> pEntity( entities.AddEntity("Widget", bees::Entities::AnonymousPersistentID) );
	pEntity->Detach();

	lean::scoped_ptr<beScene::MeshController> pMeshController( meshes.AddController() );
	pMeshController->SetMesh(renderableMesh);
	pEntity->AddController(pMeshController.move_ptr());

	return pEntity.transfer();
}

/// Allocates a widget ID.
inline uint4 reserveWidgetID()
{
	static uint4 nextWidgetID = static_cast<uint4>(-1) / 4 * 3;
	return nextWidgetID++;
}

#endif
