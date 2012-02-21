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

/// Creates a character controller.
BE_PHYSICS_PX_API physx::PxControllerManager* CreateCharacterScene(physx::PxPhysics *pPhysics);

// Prototypes
class Device;

/// Character scene implementation.
class CharacterScenePX : public lean::noncopyable_chain< beCore::TransitiveWrapper<physx::PxControllerManager, CharacterScenePX> >, public CharacterScene
{
private:
	scoped_pxptr_t<physx::PxControllerManager>::t m_pController;

public:
	/// Constructor.
	BE_PHYSICS_PX_API CharacterScenePX(Device &device);
	/// Constructor.
	BE_PHYSICS_PX_API CharacterScenePX(physx::PxControllerManager *pController);
	/// Destructor.
	BE_PHYSICS_PX_API ~CharacterScenePX();

	/// Updates all character controllres.
	BE_PHYSICS_PX_API void Update();

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PXImplementation; }

	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxControllerManager*const& GetInterface() { return m_pController.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE const physx::PxControllerManager*const& GetInterface() const { return m_pController.get(); }
	/// Gets the PhysX interface.
	LEAN_INLINE physx::PxControllerManager*const& GetController() { return m_pController.get(); }
};

template <> struct ToImplementationPX<CharacterScene> { typedef CharacterScenePX Type; };

} // namespace

#endif