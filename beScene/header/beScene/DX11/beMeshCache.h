/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MESH_CACHE_DX11
#define BE_SCENE_MESH_CACHE_DX11

#include "../beScene.h"
#include "../beMeshCache.h"
#include <lean/pimpl/pimpl_ptr.h>
#include <beGraphics/Any/beAPI.h>

namespace beScene
{
namespace DX11
{

/// Mesh cache implementation.
class MeshCache : public beScene::MeshCache
{
public:
	struct M;
	friend struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_SCENE_API MeshCache(ID3D11Device *pDevice, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_SCENE_API ~MeshCache();

	/// Adds a named mesh.
	BE_SCENE_API void SetMeshName(const lean::utf8_ntri &name, MeshCompound *pMesh);
	/// Adds a named mesh.
	BE_SCENE_API MeshCompound* SetMeshName(const lean::utf8_ntri &name, beScene::Mesh *pMesh);
	/// Gets a mesh from the given name.
	BE_SCENE_API MeshCompound* GetMeshByName(const lean::utf8_ntri &name, bool bThrow = false) const;

	/// Gets a mesh from the given file.
	BE_SCENE_API beScene::MeshCompound* GetMesh(const lean::utf8_ntri &file);
	
	/// Gets the file (or name) of the given mesh.
	BE_SCENE_API utf8_ntr GetFile(const MeshCompound *pMesh, bool *pIsFile = nullptr) const;

	/// Notifies dependent listeners about dependency changes.
	BE_SCENE_API void NotifyDependents();
	/// Gets the dependencies registered for the given mesh.
	BE_SCENE_API beCore::Dependency<beScene::MeshCompound*>* GetDependencies(const beScene::MeshCompound *pMesh);

	/// Gets the path resolver.
	BE_SCENE_API const beCore::PathResolver& GetPathResolver() const;

	/// Gets the device.
	ID3D11Device* GetDevice() const;

	/// Gets the implementation identifier.
	LEAN_INLINE beGraphics::ImplementationID GetImplementationID() const { return beGraphics::DX11Implementation; }
};

} // namespace

using beGraphics::DX11::ToImpl;

} // namespace

namespace beGraphics
{
	namespace DX11
	{
		template <> struct ToImplementationDX11<beScene::MeshCache> { typedef beScene::DX11::MeshCache Type; };
	} // namespace
} // namespace

#endif