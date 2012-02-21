/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beCharacter.h"
#include "bePhysics/PX/beRigidActors.h"

namespace bePhysics
{

// Creates a character from the given capsule.
lean::resource_ptr<Character, true> CreateCharacter(CharacterScene &charScene, Scene &scene, const CharacterDesc &desc,
	const Material &material, CharacterCallback *pCallback)
{
	return lean::bind_resource<Character>(
		new CharacterPX(charScene, scene, desc, material, pCallback) );
}

// Sets filter data for the given character.
void SetSimulationFilterData(Character &character, uint4 groupFlags, uint4 typeFlags, uint4 ID)
{
	SetSimulationFilterData( *ToImpl(character)->getActor(), groupFlags, typeFlags, ID); 
}

// Gets filter data from the given character.
void GetSimulationFilterData(const Character &character, int4 &groupFlags, uint4 &typeFlags, uint4 &ID)
{
	GetSimulationFilterData( *ToImpl(character)->getActor(), groupFlags, typeFlags, ID); 
}

} // namespace