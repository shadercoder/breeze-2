/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_CONTROLLER
#define BE_ENTITYSYSTEM_CONTROLLER

#include "beEntitySystem.h"
#include <beCore/beShared.h>
#include <beCore/beExchangeContainers.h>
#include "beAttachable.h"
#include <beCore/beReflectionPropertyProvider.h>

namespace beEntitySystem
{

/// Controller base class.
class LEAN_INTERFACE Controller : public beCore::Resource, public Attachable, public beCore::ReflectionPropertyProvider
{
protected:
	LEAN_INLINE Controller& operator =(const Controller &right) { return *this; }

public:
	/// Constructor.
	BE_ENTITYSYSTEM_API Controller();
	/// Destructor.
	BE_ENTITYSYSTEM_API virtual ~Controller();

	/// Synchronizes this controller with the entity controlled.
	virtual void Synchronize() { }

	/// Gets the reflection properties.
	BE_ENTITYSYSTEM_API static Properties GetControllerProperties();
	/// Gets the reflection properties.
	BE_ENTITYSYSTEM_API Properties GetReflectionProperties() const;

	/// Gets the controller type.
	virtual utf8_ntr GetType() const = 0;
};

} // namespace

#endif