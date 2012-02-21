/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_CONTROLLERDRIVEN
#define BE_ENTITYSYSTEM_CONTROLLERDRIVEN

#include "beEntitySystem.h"
#include <lean/smart/resource_ptr.h>
#include <beCore/beExchangeContainers.h>
#include "beAttachable.h"

namespace beEntitySystem
{

// Prototypes.
class Controller;

/// Controller-driven base class.
class ControllerDriven : public Attachable
{
public:
	/// Controller vector type.
	typedef std::vector< lean::resource_ptr<Controller> > controller_vector;

private:
	controller_vector m_controllers;

protected:
	ControllerDriven& operator =(const ControllerDriven&) { return *this; }

public:
	/// Controller range type.
	typedef lean::range<Controller *const *> Controllers;

	/// Constructor.
	BE_ENTITYSYSTEM_API ControllerDriven();
	/// Destructor.
	BE_ENTITYSYSTEM_API ~ControllerDriven();

	/// Adds the given controller.
	BE_ENTITYSYSTEM_API void AddController(Controller *pController);
	/// Removes the given controller.
	BE_ENTITYSYSTEM_API void RemoveController(Controller *pController);
	/// Gets a vector of all controllers.
	LEAN_INLINE Controllers GetControllers() const { return Controllers(&m_controllers.front().get(), &m_controllers.back().get() + 1); }
	/// Clears all controllers.
	BE_ENTITYSYSTEM_API void ClearControllers();

	/// Synchronizes all controllers.
	BE_ENTITYSYSTEM_API void Synchronize();

	/// Attaches all controllers to their simulations / data sources.
	BE_ENTITYSYSTEM_API void Attach();
	/// Detaches all controllers from their simulations / data sources.
	BE_ENTITYSYSTEM_API void Detach();
};

} // namespace

#endif