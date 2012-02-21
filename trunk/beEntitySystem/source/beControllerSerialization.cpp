/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beControllerSerialization.h"
#include "beEntitySystem/beController.h"
#include "beEntitySystem/beControllerSerializer.h"

namespace beEntitySystem
{
	// Instantiate controller serialization
	template class Serialization<Controller, ControllerSerializer>;
}
// Link controller serialization
#include "beSerialization.cpp"

namespace beEntitySystem
{

// Gets the controller serialization register.
ControllerSerialization& GetControllerSerialization()
{
	static ControllerSerialization manager;
	return manager;
}

} // namespace
