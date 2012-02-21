/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/DX11/beMeshCache.h"
#include "beScene/DX11/beMesh.h"
#include "beScene/beMeshSerialization.h"

#include <beGraphics/DX11/beDevice.h>

#include <lean/smart/cloneable_obj.h>
#include <lean/smart/com_ptr.h>
#include <unordered_map>

#include <beCore/beFileWatch.h>
#include <beCore/beDependenciesImpl.h>

#include <lean/io/filesystem.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beScene
{
namespace DX11
{

/// Mesh cache implementation
struct MeshCache::M
{
	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	lean::com_ptr<ID3D11Device> pDevice;

	/// Mesh
	struct ObservedMesh : public beCore::FileObserver
	{
		lean::resource_ptr<MeshCompound> pMesh;

		M *pCache;					// WARNING: nullptr, if not a file
		const utf8_string *file;	// name, if not a file

		beCore::Dependency<beScene::MeshCompound*> *dependency;

		/// Constructor.
		ObservedMesh(lean::resource_ptr<MeshCompound> pMesh, M *pCache)
			: pMesh(pMesh.transfer()),
			pCache(pCache),
			file(),
			dependency() { }

		/// Method called whenever an observed mesh has changed.
		void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision);
	};

	typedef std::unordered_map<utf8_string, ObservedMesh> mesh_map;
	mesh_map meshes;
	mesh_map namedMeshes;

	typedef std::unordered_map<const beScene::MeshCompound*, ObservedMesh*> mesh_info_map;
	mesh_info_map meshInfo;

	beCore::FileWatch fileWatch;

	beCore::DependenciesImpl<beScene::MeshCompound*> dependencies;

	/// Constructor.
	M(ID3D11Device *pDevice, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: resolver(resolver),
		provider(contentProvider),
		pDevice(pDevice)
	{
		LEAN_ASSERT(pDevice != nullptr);
	}
};

// Loads a mesh from the given file.
lean::resource_ptr<MeshCompound, true> LoadMesh(MeshCache::M &m, const lean::utf8_ntri &file, beScene::MeshCache *pCache)
{
	lean::com_ptr<beCore::Content> content = m.provider->GetContent(file);
	return LoadMeshes(
			content->Bytes(), content->Size(),
			beGraphics::DX11::Device(m.pDevice), pCache
		);
}

// Constructor.
MeshCache::MeshCache(ID3D11Device *pDevice, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(pDevice, resolver, contentProvider) )
{
}

// Destructor.
MeshCache::~MeshCache()
{
}

// Adds a named mesh.
void MeshCache::SetMeshName(const lean::utf8_ntri &nameRange, MeshCompound *pMesh)
{
	utf8_string name = nameRange.to<utf8_string>();

	// Try to find cached mesh
	M::mesh_map::iterator itMesh = m->namedMeshes.find(name);

	if (itMesh == m->namedMeshes.end())
	{
		LEAN_LOG("Adding mesh \"" << name.c_str() << "\"");
		
		// Insert mesh into cache
		itMesh = m->namedMeshes.insert(
				M::mesh_map::value_type(
					name,
					// WARNING: Back pointer only valid for files
					M::ObservedMesh( pMesh, nullptr )
				)
			).first;
		itMesh->second.file = &itMesh->first;

		// Link back to mesh info
		m->meshInfo[itMesh->second.pMesh] = &itMesh->second;

		// Allow for monitoring of dependencies
		itMesh->second.dependency = m->dependencies.AddDependency(itMesh->second.pMesh);
	}
	else if (itMesh->second.pMesh != pMesh)
	{
		itMesh->second.pMesh = pMesh;

		// Notify dependent listeners
		m->dependencies.DependencyChanged(
			itMesh->second.dependency,
			pMesh);
	}
}

// Adds a named mesh.
beScene::MeshCompound* MeshCache::SetMeshName(const lean::utf8_ntri &name, beScene::Mesh *pMesh)
{
	lean::resource_ptr<beScene::MeshCompound> pCompound = lean::new_resource<beScene::MeshCompound>(&pMesh, &pMesh + 1, this);
	SetMeshName(name, pCompound);
	return pCompound;
}

// Gets a mesh from the given name.
beScene::MeshCompound* MeshCache::GetMeshByName(const lean::utf8_ntri &name, bool bThrow) const
{
	// Try to find cached mesh
	M::mesh_map::const_iterator itMesh = m->namedMeshes.find( name.to<utf8_string>() );

	if (itMesh != m->namedMeshes.end())
		return itMesh->second.pMesh;
	else if (!bThrow)
		return nullptr;
	else
	{
		LEAN_THROW_ERROR_CTX("No mesh stored under the given name", name.c_str());
		LEAN_ASSERT(false);
	}
}

// Gets a mesh from the given file.
beScene::MeshCompound* MeshCache::GetMesh(const lean::utf8_ntri &unresolvedFile)
{
	// Get absolute path
	beCore::Exchange::utf8_string excPath = m->resolver->Resolve(unresolvedFile, true);
	utf8_string path(excPath.begin(), excPath.end());

	// Try to find cached mesh
	M::mesh_map::iterator itMesh = m->meshes.find(path);

	if (itMesh == m->meshes.end())
	{
		// WARNING: Resource pointer invalid after transfer
		{
			LEAN_LOG("Attempting to load mesh \"" << path << "\"");
			
			lean::resource_ptr<MeshCompound> pMesh = LoadMesh(*m, path, this);
			
			LEAN_LOG("Mesh \"" << unresolvedFile.c_str() << "\" created successfully");

			// Insert mesh into cache
			itMesh = m->meshes.insert(
					M::mesh_map::value_type(
						path,
						M::ObservedMesh( pMesh.transfer(), m.getptr() )
					)
				).first;
			itMesh->second.file = &itMesh->first;
		}

		// Link back to mesh info
		m->meshInfo[itMesh->second.pMesh] = &itMesh->second;

		// Allow for monitoring of dependencies
		itMesh->second.dependency = m->dependencies.AddDependency(itMesh->second.pMesh);

		// Watch mesh changes
		m->fileWatch.AddObserver(path, &itMesh->second);
	}

	return itMesh->second.pMesh;
}

// Gets the file (or name) of the given mesh.
utf8_ntr MeshCache::GetFile(const MeshCompound *pMesh, bool *pIsFile) const
{
	utf8_ntr file("");

	M::mesh_info_map::const_iterator itInfo = m->meshInfo.find(pMesh);

	if (itInfo != m->meshInfo.end())
	{
		file = utf8_ntr(*itInfo->second->file);

		if (pIsFile)
			// Back pointer only valid for files
			*pIsFile = (itInfo->second->pCache != nullptr);
	}
	
	return file;
}

// Notifies dependent listeners about dependency changes.
void MeshCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

// Gets the dependencies registered for the given mesh.
beCore::Dependency<beScene::MeshCompound*>* MeshCache::GetDependencies(const beScene::MeshCompound *pMesh)
{
	M::mesh_info_map::iterator itMesh = m->meshInfo.find(pMesh);

	return (itMesh != m->meshInfo.end())
		? itMesh->second->dependency
		: nullptr;
}

// Method called whenever an observed mesh has changed.
void MeshCache::M::ObservedMesh::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	// WARNING: Back pointer only valid for files
	LEAN_ASSERT_NOT_NULL( this->pCache );

	// MONITOR: Non-atomic asynchronous assignment!
	this->pMesh = LoadMesh(*this->pCache, *this->file, this->pMesh->GetCache());

	// Notify dependent listeners
	this->pCache->dependencies.DependencyChanged(this->dependency, this->pMesh);
}

// Gets the device.
ID3D11Device* MeshCache::GetDevice() const
{
	return m->pDevice;
}

} // namespace

// Creates a new mesh cache.
lean::resource_ptr<MeshCache, true> CreateMeshCache(const beGraphics::Device &device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return lean::bind_resource<MeshCache>(
			new DX11::MeshCache( ToImpl(device), resolver, contentProvider )
		);
}

} // namespace