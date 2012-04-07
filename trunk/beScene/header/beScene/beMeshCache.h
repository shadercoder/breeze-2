/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MESH_CACHE
#define BE_SCENE_MESH_CACHE

#include "beScene.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include "beMeshCompound.h"
#include <beGraphics/beDevice.h>
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>
#include <beCore/beDependencies.h>

namespace beScene
{

/// Mesh cache base.
class MeshCache : public lean::noncopyable, public beCore::Resource, public beGraphics::Implementation
{
public:
	virtual ~MeshCache() throw() { }

	/// Adds a named mesh.
	virtual void SetMeshName(const lean::utf8_ntri &name, MeshCompound *pMesh) = 0;
	/// Adds a named mesh.
	virtual MeshCompound* SetMeshName(const lean::utf8_ntri &name, beScene::Mesh *pMesh) = 0;
	/// Gets a mesh from the given name.
	virtual MeshCompound* GetMeshByName(const lean::utf8_ntri &name, bool bThrow = false) const = 0;

	/// Gets a mesh from the given file.
	virtual MeshCompound* GetMesh(const lean::utf8_ntri &file) = 0;

	/// Gets the file (or name) of the given mesh.
	virtual utf8_ntr GetFile(const MeshCompound *pMesh, bool *pIsFile = nullptr) const = 0;

	/// Notifies dependent listeners about dependency changes.
	virtual void NotifyDependents() = 0;
	/// Gets the dependencies registered for the given mesh.
	virtual beCore::Dependency<MeshCompound*>* GetDependencies(const MeshCompound *pMesh) = 0;

	/// Gets the path resolver.
	virtual const beCore::PathResolver& GetPathResolver() const = 0;
};

/// Creates a new mesh cache.
BE_SCENE_API lean::resource_ptr<MeshCache, true> CreateMeshCache(const beGraphics::Device &device,
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

} // namespace

#endif