/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_ENTITYCONTROLLER
#define BE_ENTITYSYSTEM_ENTITYCONTROLLER

#include "beEntitySystem.h"
#include "beController.h"

namespace beEntitySystem
{

// Prototypes
class Entity;

/// Entity controller base class.
class LEAN_INTERFACE EntityController : public Controller
{
protected:
	/// Controlled entity.
	Entity *const m_pEntity;

public:
	/// Constructor.
	LEAN_INLINE EntityController(Entity *pEntity)
		: m_pEntity( LEAN_ASSERT_NOT_NULL(pEntity) ) { }

	/// Gets the controlled entity.
	LEAN_INLINE Entity* GetEntity() const { return m_pEntity; }
};

} // namespace

#endif