/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MESH_CONTROLLER
#define BE_SCENE_MESH_CONTROLLER

#include "beScene.h"
#include "beScene/beEntityController.h"
#include "beRenderable.h"
#include <beEntitySystem/beSynchronized.h>

#include <lean/smart/resource_ptr.h>
#include <lean/containers/dynamic_array.h>

#include "beMath/beSphereDef.h"

namespace beScene
{

// Prototypes
class Mesh;
class SceneController;
class DynamicScenery;
class RenderableMaterial;
class RenderContext;

/// Material-driven interface.
class MeshController : public EntityController, public Renderable, public beEntitySystem::Synchronized
{
public:
	struct Subset;
	typedef std::vector<Subset> subset_vector;

	struct Pass;
	typedef lean::dynamic_array<Pass> pass_vector;

	struct SharedData;

	struct M
	{
		DynamicScenery *pScenery;

		subset_vector subsets;

		beMath::fsphere3 localBounds;
		pass_vector passes;

		beMath::fsphere3 *pBounds;
		SharedData *pSharedData;
		
		bool bRendered;
		bool bSynchronized;
		bool bVisible;

		/// Constructor.
		M(DynamicScenery *pScenery);
	};

private:
	M m;

public:
	/// Constructor.
	BE_SCENE_API MeshController(beEntitySystem::Entity *pEntity, SceneController *pScene, DynamicScenery *pScenery);
	/// Destructor.
	BE_SCENE_API ~MeshController();

	/// Gets the sort index.
	BE_SCENE_API uint4 GetSortIndex() const;
	/// Gets the number of passes.
	BE_SCENE_API uint4 GetPassCount() const;

	/// Called when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator);
	/// Called for each pass when a renderable is attached to a scenery.
	BE_SCENE_API void Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator);
	/// Called when a renderable is detached from a scenery.
	BE_SCENE_API void Detached(DynamicScenery *pScenery);

	/// Renders the mesh.
	BE_SCENE_API static void Render(const RenderJob &job, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context);

	/// Gets the number of subsets.
	BE_SCENE_API uint4 GetSubsetCount() const;
	/// Gets the n-th mesh.
	BE_SCENE_API const Mesh* GetMesh(uint4 subsetIdx) const;
	/// Adds the given mesh setting the given material.
	BE_SCENE_API uint4 AddMeshWithMaterial(const Mesh *pMesh, const RenderableMaterial *pMaterial);
	/// Sets the n-th mesh.
	BE_SCENE_API void SetMeshWithMaterial(uint4 subsetIdx, const Mesh *pMesh, const RenderableMaterial *pMaterial);
	/// Removes the given mesh with the given material.
	BE_SCENE_API void RemoveMeshWithMaterial(const Mesh *pMesh, const RenderableMaterial *pMaterial = nullptr);
	/// Removes the n-th subset.
	BE_SCENE_API void RemoveSubset(uint4 subsetIdx);
	/// Updates the controller from all subsets (e.g. when controller is not attached and thus not updated automatically).
	BE_SCENE_API void UpdateFromSubsets();

	/// Sets the material.
	BE_SCENE_API void SetMaterial(const RenderableMaterial *pMaterial);
	/// Sets the n-th material.
	BE_SCENE_API void SetMaterial(uint4 subsetIdx, const RenderableMaterial *pMaterial);
	/// Gets the n-th material.
	BE_SCENE_API const RenderableMaterial* GetMaterial(uint4 subsetIdx) const;

	/// Attaches this controller to the scenery.
	BE_SCENE_API void Attach();
	/// Detaches this controller from the scenery.
	BE_SCENE_API void Detach();

	/// Synchronizes this controller with the controlled entity.
	BE_SCENE_API void Synchronize();
	/// Synchronizes this controller with the controlled entity.
	BE_SCENE_API void Flush();

	/// Sets the visibility.
	LEAN_INLINE void SetVisible(bool bVisible) { m.bVisible = bVisible; }
	/// Gets the visibility.
	LEAN_INLINE bool IsVisible() const { return m.bVisible; }

	/// Sets the local bounding sphere.
	LEAN_INLINE void SetLocalBounds(const beMath::fsphere3 &bounds) { m.localBounds = bounds; }
	/// Gets the local bounding sphere.
	LEAN_INLINE const beMath::fsphere3& GetLocalBounds() { return m.localBounds; }

	/// Gets the number of child components.
	BE_SCENE_API uint4 GetComponentCount() const;
	/// Gets the name of the n-th child component.
	BE_SCENE_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_SCENE_API const ReflectedComponent* GetReflectedComponent(uint4 idx) const;

	/// Gets the type of the n-th child component.
	BE_SCENE_API beCore::Exchange::utf8_string GetComponentType(uint4 idx) const;
	/// Gets the n-th component.
	BE_SCENE_API lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const;
	/// Returns true, if the n-th component can be replaced.
	BE_SCENE_API bool IsComponentReplaceable(uint4 idx) const;
	/// Sets the n-th component.
	BE_SCENE_API void SetComponent(uint4 idx, const lean::any &pComponent);

	/// Gets the scenery.
	LEAN_INLINE DynamicScenery* GetScenery() const { return m.pScenery; }

	/// Gets the controller type.
	BE_SCENE_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

class MeshCompound;

/// Adds all meshes in the given mesh compound to the given mesh controller using the given material.
BE_SCENE_API void AddMeshes(MeshController &controller, const MeshCompound &compound, const RenderableMaterial *pMaterial);

class ResourceManager;
class EffectDrivenRenderer;

/// Sets the default mesh effect file.
BE_SCENE_API void SetMeshDefaultEffect(const utf8_ntri &file);
/// Gets the default mesh effect file.
BE_SCENE_API beCore::Exchange::utf8_string GetMeshDefaultEffect();
/// Gets the default material for meshes.
BE_SCENE_API RenderableMaterial* GetMeshDefaultMaterial(ResourceManager &resources, EffectDrivenRenderer &renderer);


} // namespace

#endif