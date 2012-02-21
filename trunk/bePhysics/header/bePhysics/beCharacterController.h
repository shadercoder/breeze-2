/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_CHARACTERCONTROLLER
#define BE_PHYSICS_CHARACTERCONTROLLER

#include "bePhysics.h"
#include "beEntityController.h"
#include <beEntitySystem/beSynchronized.h>
#include <lean/smart/resource_ptr.h>
#include <beMath/beVectorDef.h>

namespace bePhysics
{

// Prototypes
class CharacterSceneController;
class Character;
class Material;

/// Character controller class.
class CharacterController : public beEntitySystem::EntityController, public beEntitySystem::Synchronized
{
private:
	bool m_bAttached;
	bool m_bDynamic;

	beMath::fvec3 m_lastScaling;

protected:
	/// Scene.
	const lean::resource_ptr<CharacterSceneController> m_pScene;

	/// Material
	lean::resource_ptr<const Material> m_pMaterial;

	/// Actor.
	lean::resource_ptr<Character> m_pActor;

public:
	/// Constructor.
	BE_PHYSICS_API CharacterController(beEntitySystem::Entity *pEntity, CharacterSceneController *pScene, Character *pActor,
		const Material *pMaterial = nullptr);
	/// Destructor.
	BE_PHYSICS_API ~CharacterController();

	/// Sets the material.
	BE_PHYSICS_API void SetMaterial(const Material *pMaterial);
	/// Gets the material.
	LEAN_INLINE const Material* GetMaterial() const { return m_pMaterial; }

	/// Sets the mass.
	BE_PHYSICS_API void SetMass(float mass);
	/// Sets the mass.
	BE_PHYSICS_API void SetMassAndInertia(float mass);
	/// Sets the mass.
	BE_PHYSICS_API float GetMass() const;

	/// Forces this controller into synchronization with the simulation.
	BE_PHYSICS_API void Synchronize();

	/// Synchronizes this controller with the simulation.
	BE_PHYSICS_API void Flush();
	/// Synchronizes the simulation with this controller.
	BE_PHYSICS_API void Fetch();

	/// Attaches this controller to the scene.
	BE_PHYSICS_API void Attach();
	/// Detaches this controller from the scene.
	BE_PHYSICS_API void Detach();

	/// Converts the controlled actor between non-kinematic and kinematic.
	BE_PHYSICS_API void SetDynamic(bool bDynamic);
	/// Checks if the actor is currently in kinematic mode.
	LEAN_INLINE bool IsDynamic() const { return m_bDynamic; }

	/// Gets the actor.
	LEAN_INLINE Character* GetCharacter() const { return m_pActor; }

	/// Gets the controller type.
	BE_PHYSICS_API static utf8_ntr GetControllerType();
	/// Gets the controller type.
	utf8_ntr GetType() const { return GetControllerType(); }
};

} // namespace

#endif