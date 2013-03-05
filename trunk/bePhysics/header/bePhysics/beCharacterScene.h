/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_CHARACTER_SCENE
#define BE_PHYSICS_CHARACTER_SCENE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Character scene interface.
class CharacterScene : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE CharacterScene& operator =(const CharacterScene&) { return *this; }

public:
	virtual ~CharacterScene() throw() { }

	/// Updates all character controllres.
	virtual void Update() = 0;
};

class Device;

/// Creates a character scene.
BE_PHYSICS_API lean::resource_ptr<CharacterScene, true> CreateCharacterScene(Device *device);

} // namespace

#endif