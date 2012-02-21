/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beControllerDriven.h"
#include "beEntitySystem/beController.h"
#include <lean/functional/algorithm.h>
#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Constructor.
ControllerDriven::ControllerDriven()
{
}

// Destructor.
ControllerDriven::~ControllerDriven()
{
}

// Adds the given controller.
void ControllerDriven::AddController(Controller *pController)
{
	if (!pController)
	{
		LEAN_LOG_ERROR_MSG("pController may not be nullptr");
		return;
	}

	m_controllers.push_back(pController);
}

// Removes the given controller.
void ControllerDriven::RemoveController(Controller *pController)
{
	lean::remove_ordered(m_controllers, pController);
}

// Clears all controllers.
void ControllerDriven::ClearControllers()
{
	m_controllers.clear();
}

// Synchronizes all controllers.
void ControllerDriven::Synchronize()
{
	for (controller_vector::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
		(*it)->Synchronize();
}

// Attaches all controllers to their simulations / data sources.
void ControllerDriven::Attach()
{
	for (controller_vector::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
		(*it)->Attach();
}

// Detaches all controllers from their simulations / data sources.
void ControllerDriven::Detach()
{
	for (controller_vector::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
		(*it)->Detach();
}

} // namespace
