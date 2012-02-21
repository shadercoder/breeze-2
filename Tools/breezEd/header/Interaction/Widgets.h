#ifndef WIDGETS_H
#define WIDGETS_H

#include "breezEd.h"

#include <beScene/beSceneController.h>

#include <beEntitySystem/beEntity.h>
#include <beScene/beMeshController.h>

#include <beScene/beResourceManager.h>
#include <beScene/beMeshCache.h>
#include <beGraphics/beEffectCache.h>
#include <beScene/beMaterial.h>
#include <beScene/beRenderableMaterial.h>

#include <beScene/beEffectDrivenRenderer.h>

#include <lean/smart/resource_ptr.h>

/// Creates a transform widget entity of the given color and mesh.
inline lean::resource_ptr<beEntitySystem::Entity, true> createWidgetMesh(uint4 ID, const lean::utf8_ntri &mesh, const beMath::fvec4 &color,
	beScene::SceneController *pScene, beScene::ResourceManager &resources, beScene::EffectDrivenRenderer &renderer)
{
	lean::resource_ptr<beEntitySystem::Entity> pEntity = lean::bind_resource( new beEntitySystem::Entity("Widget") );
	pEntity->SetID(ID);

	beScene::MeshCompound *pMeshes = resources.MeshCache()->GetMesh(mesh);

	lean::resource_ptr<beScene::MeshController> pMeshController = lean::new_resource<beScene::MeshController>( pEntity, pScene, pScene->GetScenery() );

	// TODO: NO HARD-CODED PATHS
	lean::resource_ptr<beScene::Material> pMaterial = lean::new_resource<beScene::Material>(
			resources.EffectCache()->GetEffect("Materials/Widget.fx", nullptr, 0),
			*resources.EffectCache()
		);

	// TODO: How to handle material layers?
	beGraphics::Setup &materialSetup = *pMaterial->GetSetup(0);
	materialSetup.SetProperty( materialSetup.GetPropertyID("Color"), color.data(), 4 );
	
	lean::resource_ptr<beScene::RenderableMaterial> pRenderableMaterial = lean::new_resource<beScene::RenderableMaterial>(pMaterial, *renderer.RenderableDrivers());
	AddMeshes(*pMeshController, *pMeshes, pRenderableMaterial);

	pEntity->AddController(pMeshController);

	return pEntity.transfer();
}

/// Allocates a widget ID.
inline uint4 reserveWidgetID()
{
	static uint4 nextWidgetID = static_cast<uint4>(-1) / 4 * 3;
	return nextWidgetID++;
}

#endif
