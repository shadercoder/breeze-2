/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beScene.h"

namespace bePhysics
{

// Creates a physics scene.
lean::resource_ptr<Scene, true> CreateScene(Device &device, const SceneDesc &desc)
{
	return lean::bind_resource( new ScenePX(device, desc) );
}

} // namespace