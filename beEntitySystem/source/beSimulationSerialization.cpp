/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beSimulationSerialization.h"
#include "beEntitySystem/beSimulation.h"

namespace beEntitySystem
{
	// Instantiate simulation serialization
	template class Serialization<Simulation>;
}
// Link simulation serialization
#include "beSerialization.cpp"

namespace beEntitySystem
{

// Gets the simulation serialization register.
SimulationSerialization& GetSimulationSerialization()
{
	static SimulationSerialization manager;
	return manager;
}

} // namespace
