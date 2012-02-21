/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beController.h"

namespace beEntitySystem
{

// Constructor.
Controller::Controller()
{
}

// Destructor.
Controller::~Controller()
{
}

// Gets the reflection properties.
Controller::Properties Controller::GetControllerProperties()
{
	return Properties();
}

// Gets the reflection properties.
Controller::Properties Controller::GetReflectionProperties() const
{
	return Properties();
}

} // namespace
