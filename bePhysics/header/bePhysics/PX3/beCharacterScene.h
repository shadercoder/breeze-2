/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_CHARACTER_SCENE_PX
#define BE_PHYSICS_CHARACTER_SCENE_PX

#include "bePhysics.h"
#include "../beCharacterScene.h"
#include <beCore/beWrapper.h>
#include <lean/tags/noncopyable.h>
#include "beCharactersAPI.h"

namespace bePhysics
{

// Prototypes
class Device;

namespace PX3
{

/// Creates a character controller.
BE_PHYSICS_PX_API physx::PxControllerManager* CreateCharacterScene(physx::PxPhysics *physics);

/// Character scene implementation.
class CharacterScene : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxControllerManager, CharacterScene> >, public bePhysics::CharacterScene
{
private:
	scoped_pxptr_t<physx::PxControllerManager>::t m_pController;

public:
	/// Constructor.
	BE_PHYSICS_PX_API CharacterScene(bePhysics::Device *device);
	/// Constructor.
	BE_PHYSICS_PX_API CharacterScene(physx::PxControllerManager *controller);
	/// Destructor.
	BE_PHYSICS_PX_API ~CharacterScene();

	/// Updates all character controllres.
	BE_PHYSICS_PX_API void Update();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxControllerManager*const& GetInterface() { return m_pController.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxControllerManager*const& GetInterface() const { return m_pController.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxControllerManager*const& GetController() { return m_pController.get(); }
};

template <> struct ToImplementationPX<bePhysics::CharacterScene> { typedef CharacterScene Type; };

} // namespace

} // namespace

#endif