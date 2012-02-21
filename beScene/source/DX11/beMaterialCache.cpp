/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/DX11/beMaterialCache.h"
#include "beScene/beMaterial.h"
#include "beScene/beMaterialSerialization.h"

#include <beGraphics/DX11/beDevice.h>

#include <lean/smart/cloneable_obj.h>
#include <lean/smart/resource_ptr.h>
#include <unordered_map>

#include <beCore/beFileWatch.h>
#include <beCore/beDependenciesImpl.h>

#include <lean/xml/xml_file.h>
#include <lean/io/numeric.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beScene
{
namespace DX11
{

/// Material cache implementation
struct MaterialCache::M
{
	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	lean::resource_ptr<beGraphics::EffectCache> pEffectCache;
	lean::resource_ptr<beGraphics::TextureCache> pTextureCache;

	/// Material
	struct ObservedMaterial : public beCore::FileObserver
	{
		lean::resource_ptr<Material> pMaterial;

		MaterialCache *pCache;		// WARNING: nullptr, if not a file
		const utf8_string *file;	// name, if not a file

		beCore::Dependency<beScene::Material*> *dependency;

		/// Constructor.
		ObservedMaterial(lean::resource_ptr<Material> pMaterial, MaterialCache *pCache)
			: pMaterial(pMaterial.transfer()),
			pCache(pCache),
			file(),
			dependency() { }

		/// Method called whenever an observed material has changed.
		void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision);
	};

	typedef std::unordered_map<utf8_string, ObservedMaterial> material_map;
	material_map materials;

	typedef std::unordered_map<const beScene::Material*, ObservedMaterial*> material_info_map;
	material_info_map materialInfo;

	material_map namedMaterials;

	beCore::FileWatch fileWatch;

	beCore::DependenciesImpl<beScene::Material*> dependencies;

	/// Constructor.
	M(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache,
			const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: resolver(resolver),
		provider(contentProvider),
		pEffectCache( LEAN_ASSERT_NOT_NULL(pEffectCache) ),
		pTextureCache( LEAN_ASSERT_NOT_NULL(pTextureCache) )
	{
	}
};

// Loads a material from the given file.
lean::resource_ptr<Material, true> LoadMaterial(MaterialCache::M &m, const lean::utf8_ntri &file, MaterialCache *pCache)
{
	rapidxml::xml_document<utf8_t> document;
	utf8_t *documentSource;

	// Load material source code
	{
		lean::com_ptr<beCore::Content> content = m.provider->GetContent(file);
		
		// Null-termination required :-/
		size_t documentSize = static_cast<size_t>(content->Size());
		documentSource = document.allocate_string(nullptr, documentSize + 1);
		memcpy(documentSource, content->Bytes(), documentSize);
		documentSource[documentSize] = 0;
	}

	document.parse<rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace>(documentSource);

	return beScene::LoadMaterial(document, *m.pEffectCache, *m.pTextureCache, pCache);
}

// Constructor.
MaterialCache::MaterialCache(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache,
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(pEffectCache, pTextureCache, resolver, contentProvider) )
{
}

// Destructor.
MaterialCache::~MaterialCache()
{
}

// Sets the given name for the given material.
beScene::Material* MaterialCache::SetMaterialName(const lean::utf8_ntri &nameRange, Material *pMaterial)
{
	LEAN_ASSERT_NOT_NULL(pMaterial);

	utf8_string name = nameRange.to<utf8_string>();

	// Try to find cached material
	M::material_map::iterator itMaterial = m->namedMaterials.find(name);

	if (itMaterial == m->namedMaterials.end())
	{
		LEAN_LOG("Adding material \"" << name.c_str() << "\"");

		// Insert material into cache
		itMaterial = m->namedMaterials.insert(
				M::material_map::value_type(
					name,
					// WARNING: Back pointer only valid for files
					M::ObservedMaterial( pMaterial, nullptr )
				)
			).first;
		itMaterial->second.file = &itMaterial->first;

		// Link back to material info
		m->materialInfo[itMaterial->second.pMaterial] = &itMaterial->second;

		// Allow for monitoring of dependencies
		itMaterial->second.dependency = m->dependencies.AddDependency(itMaterial->second.pMaterial);
	}
	else
	{
		itMaterial->second.pMaterial = pMaterial;

		// Notify dependent listeners
		m->dependencies.DependencyChanged(
			itMaterial->second.dependency,
			pMaterial);
	}

	return pMaterial;
}

// Gets a material by name.
beScene::Material* MaterialCache::GetMaterialByName(const lean::utf8_ntri &name, bool bThrow) const
{
	// Try to find cached material
	M::material_map::const_iterator itMaterial = m->namedMaterials.find( name.to<utf8_string>() );

	if (itMaterial != m->namedMaterials.end())
		return itMaterial->second.pMaterial;
	else if (!bThrow)
		return nullptr;
	else
	{
		LEAN_THROW_ERROR_CTX("No material stored under the given name", name.c_str());
		LEAN_ASSERT(false);
	}
}

// Gets a material from the given effect & name.
beScene::Material* MaterialCache::GetMaterial(const beGraphics::Effect *pEffect, const lean::utf8_ntri &nameRange)
{
	if (!pEffect)
		return nullptr;

	utf8_string name = nameRange.to<utf8_string>();

	// Try to find cached material
	M::material_map::iterator itMaterial = m->namedMaterials.find(name);

	if (itMaterial == m->namedMaterials.end())
	{
		// WARNING: Resource pointer invalid after transfer
		{
			LEAN_LOG("Attempting to create material \"" << name.c_str() << "\"");

			lean::resource_ptr<Material> pMaterial = lean::new_resource<Material>(pEffect, *m->pEffectCache, this);

			LEAN_LOG("Material \"" << name.c_str() << "\" created successfully");

			// Insert material into cache
			itMaterial = m->namedMaterials.insert(
					M::material_map::value_type(
						name,
						// WARNING: Back pointer only valid for files
						M::ObservedMaterial( pMaterial.transfer(), nullptr )
					)
				).first;
			itMaterial->second.file = &itMaterial->first;
		}

		// Link back to material info
		m->materialInfo[itMaterial->second.pMaterial] = &itMaterial->second;

		// Allow for monitoring of dependencies
		itMaterial->second.dependency = m->dependencies.AddDependency(itMaterial->second.pMaterial);
	}

	return itMaterial->second.pMaterial;
}

// Gets a material from the given file.
beScene::Material* MaterialCache::GetMaterial(const lean::utf8_ntri &unresolvedFile)
{
	// Get absolute path
	beCore::Exchange::utf8_string excPath = m->resolver->Resolve(unresolvedFile, true);
	utf8_string path(excPath.begin(), excPath.end());

	// Try to find cached material
	M::material_map::iterator itMaterial = m->materials.find(path);

	if (itMaterial == m->materials.end())
	{
		// WARNING: Resource pointer invalid after transfer
		{
			LEAN_LOG("Attempting to load material \"" << path << "\"");

			lean::resource_ptr<Material> pMaterial = LoadMaterial(*m, path, this);

			LEAN_LOG("Material \"" << unresolvedFile.c_str() << "\" created successfully");

			// Insert material into cache
			itMaterial = m->materials.insert(
					M::material_map::value_type(
						path,
						M::ObservedMaterial( pMaterial.transfer(), this )
					)
				).first;
			itMaterial->second.file = &itMaterial->first;
		}

		// Link back to material info
		m->materialInfo[itMaterial->second.pMaterial] = &itMaterial->second;

		// Allow for monitoring of dependencies
		itMaterial->second.dependency = m->dependencies.AddDependency(itMaterial->second.pMaterial);

		// Watch material changes
		m->fileWatch.AddObserver(path, &itMaterial->second);
	}

	return itMaterial->second.pMaterial;
}

// Gets the file (or name) of the given mesh.
utf8_ntr MaterialCache::GetFile(const Material *pMaterial, bool *pIsFile) const
{
	utf8_ntr file("");

	M::material_info_map::const_iterator itInfo = m->materialInfo.find(pMaterial);

	if (itInfo != m->materialInfo.end())
	{
		file = utf8_ntr(*itInfo->second->file);

		if (pIsFile)
			// Back pointer only valid for files
			*pIsFile = (itInfo->second->pCache != nullptr);
	}

	return file;
}

// Notifies dependent listeners about dependency changes.
void MaterialCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

// Gets the dependencies registered for the given material.
beCore::Dependency<beScene::Material*>* MaterialCache::GetDependencies(const beScene::Material *pMaterial)
{
	M::material_info_map::iterator itMaterial = m->materialInfo.find(pMaterial);

	return (itMaterial != m->materialInfo.end())
		? itMaterial->second->dependency
		: nullptr;
}

// Gets the effect cache.
beGraphics::EffectCache* MaterialCache::GetEffectCache() const
{
	return m->pEffectCache;
}

// Gets the texture cache.
beGraphics::TextureCache* MaterialCache::GetTextureCache() const
{
	return m->pTextureCache;
}

// Method called whenever an observed material has changed.
void MaterialCache::M::ObservedMaterial::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	// WARNING: Back pointer only valid for files
	LEAN_ASSERT_NOT_NULL( this->pCache );

	// MONITOR: Non-atomic asynchronous assignment!
	this->pMaterial = LoadMaterial(*this->pCache->m, *this->file, this->pCache);

	// Notify dependent listeners
	this->pCache->m->dependencies.DependencyChanged(this->dependency, this->pMaterial);
}

} // namespace

// Creates a new material cache.
lean::resource_ptr<MaterialCache, true> CreateMaterialCache(beGraphics::EffectCache *pEffectCache, beGraphics::TextureCache *pTextureCache, 
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return lean::bind_resource<MaterialCache>(
			new DX11::MaterialCache( pEffectCache, pTextureCache, resolver, contentProvider )
		);
}

} // namespace