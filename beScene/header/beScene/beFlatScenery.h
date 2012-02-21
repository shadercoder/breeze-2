/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_FLAT_SCENERY
#define BE_SCENE_FLAT_SCENERY

#include "beScene.h"
#include "beDynamicScenery.h"
#include <beCore/beShared.h>

#include <vector>
#include <lean/containers/dynamic_array.h>
#include <lean/memory/chunk_heap.h>

namespace beScene
{

/// Flat scenery container.
class FlatScenery : public beCore::Resource, public DynamicScenery
{
public:
	typedef std::vector<Renderable*> renderable_vector;

	struct Element;
	typedef lean::dynamic_array<Element> element_vector;

	typedef lean::chunk_heap<0, lean::default_heap, 0, 16>  data_heap;

private:
	renderable_vector m_renderables;

	element_vector m_elements;
	data_heap m_renderableData;
	data_heap m_passData;

	bool m_bRenderablesChanged;

public:
	/// Constructor.
	BE_SCENE_API FlatScenery();
	/// Destructor.
	BE_SCENE_API ~FlatScenery();
	
	/// Gets all renderables inside the given frustum.
	BE_SCENE_API void CullRenderables(const beMath::fplane3 *planes, AddRenderablePassFunction *pAddPass, void *pCaller) const;
	/// Gets all lights intersecting the given bounding sphere.
	BE_SCENE_API void CullLights(const beMath::fsphere3 &bounds, AddLightJobFunction *pAddLight, void *pCaller) const;

	/// Prepares the scenery for rendering access.
	BE_SCENE_API void Prepare();

	/// Adds the given render job to this scenery.
	BE_SCENE_API void AddRenderable(Renderable *pRenderable);
	/// Removes the given renderable from this scenery.
	BE_SCENE_API void RemoveRenderable(Renderable *pRenderable);
	/// Invalidates cached scenery data.
	BE_SCENE_API void InvalidateRenderables();

	/// Adds the given light to this scenery.
	BE_SCENE_API void AddLight(Light *pLight);
	/// Removes the given renderable from this scenery.
	BE_SCENE_API void RemoveLight(Light *pLight);
	/// Invalidates cached scenery data.
	BE_SCENE_API void InvalidateLights();
};

} // namespace

#endif