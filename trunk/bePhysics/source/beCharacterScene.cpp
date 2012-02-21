/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beCharacterScene.h"

namespace bePhysics
{

// Creates a character scene.
lean::resource_ptr<CharacterScene, true> CreateCharacterScene(Device &device)
{
	return lean::bind_resource<CharacterScene>(
		new CharacterScenePX(device) );
}

} // namespace