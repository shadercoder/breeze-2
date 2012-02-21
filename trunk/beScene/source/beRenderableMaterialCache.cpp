/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderableMaterialCache.h"
#include "beScene/beRenderableMaterial.h"

#include "beScene/beAbstractRenderableEffectDriver.h"
#include "beScene/beEffectBinderCache.h"

#include <lean/smart/resource_ptr.h>
#include <unordered_map>

#include <beCore/beDependenciesImpl.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beScene
{

/// Renderable material cache implementation
struct RenderableMaterialCache::M
{
	lean::resource_ptr<EffectBinderCache<AbstractRenderableEffectDriver>> pDriverCache;

	/// Material
	struct ObservedMaterial
	{
		lean::resource_ptr<RenderableMaterial> pMaterial;

		beCore::Dependency<beScene::RenderableMaterial*> *dependency;

		/// Constructor.
		ObservedMaterial(lean::resource_ptr<RenderableMaterial> pMaterial)
			: pMaterial(pMaterial.transfer()),
			dependency() { }
	};

	typedef std::unordered_map<beScene::Material*, ObservedMaterial> material_map;
	material_map materials;

	typedef std::unordered_map<const RenderableMaterial*, ObservedMaterial*> material_info_map;
	material_info_map materialInfo;

	beCore::DependenciesImpl<beScene::RenderableMaterial*> dependencies;

	/// Constructor.
	M(EffectBinderCache<AbstractRenderableEffectDriver> *pDriverCache)
		: pDriverCache( LEAN_ASSERT_NOT_NULL(pDriverCache) )
	{
	}
};

// Constructor.
RenderableMaterialCache::RenderableMaterialCache(EffectBinderCache<AbstractRenderableEffectDriver> *pDriverCache)
	: m( new M(pDriverCache) )
{
}

// Destructor.
RenderableMaterialCache::~RenderableMaterialCache()
{
}

// Gets a material from the given effect & name.
beScene::RenderableMaterial* RenderableMaterialCache::GetMaterial(beScene::Material *pMaterial)
{
	if (!pMaterial)
		return nullptr;

	// Get cached material
	M::material_map::iterator itMaterial = m->materials.find(pMaterial);
	
	if (itMaterial == m->materials.end())
	{
		// WARNING: Resource pointer invalid after transfer
		{
			lean::resource_ptr<RenderableMaterial> pRenderableMaterial =
				lean::new_resource<RenderableMaterial>(pMaterial, *m->pDriverCache);
			
			// Insert material into cache
			itMaterial = m->materials.insert(
					M::material_map::value_type(
						pMaterial,
						M::ObservedMaterial(pRenderableMaterial.transfer())
					)
				).first;
		}

		// Link back to material info
		m->materialInfo[itMaterial->second.pMaterial] = &itMaterial->second;

		// Allow for monitoring of dependencies
		itMaterial->second.dependency = m->dependencies.AddDependency(itMaterial->second.pMaterial);
	}

	return itMaterial->second.pMaterial;
}

// Notifies dependent listeners about dependency changes.
void RenderableMaterialCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

// Gets the dependencies registered for the given material.
beCore::Dependency<beScene::RenderableMaterial*>* RenderableMaterialCache::GetDependencies(const beScene::RenderableMaterial *pMaterial)
{
	M::material_info_map::iterator itMaterial = m->materialInfo.find(pMaterial);

	return (itMaterial != m->materialInfo.end())
		? itMaterial->second->dependency
		: nullptr;
}

// Creates a new material cache.
lean::resource_ptr<RenderableMaterialCache, true> CreateRenderableMaterialCache(EffectBinderCache<AbstractRenderableEffectDriver> *pDriverCache)
{
	return lean::bind_resource<RenderableMaterialCache>(
			new RenderableMaterialCache( pDriverCache )
		);
}

} // namespace
