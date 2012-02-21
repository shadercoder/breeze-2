/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_SCENERY
#define BE_SCENE_SCENERY

#include "beScene.h"
#include "beRenderJob.h"
#include <beCore/beQueryResult.h>
#include <beMath/bePlaneDef.h>
#include <beMath/beSphereDef.h>

namespace beScene
{

// Prototypes
class Renderable;
struct RenderableData;
struct RenderablePass;
class Light;
struct LightJob;

/// Add pass function type.
typedef void (AddRenderablePassFunction)(void *pCaller, const Renderable *pRenderable, const RenderableData &renderableData);
/// Add light function type.
typedef void (AddLightJobFunction)(void *pCaller, const Light *pLight, const LightJob &lightJob);

/// Scenery interface.
class Scenery
{
protected:
	Scenery& operator =(const Scenery&) { return *this; }
	~Scenery() { }

public:
	/// Gets all renderables inside the given frustum.
	virtual void CullRenderables(const beMath::fplane3 *planes, AddRenderablePassFunction *pAddPass, void *pCaller) const = 0;
	/// Gets all lights intersecting the given bounding sphere.
	virtual void CullLights(const beMath::fsphere3 &bounds, AddLightJobFunction *pAddLight, void *pCaller) const = 0;
};

} // namespace

#endif