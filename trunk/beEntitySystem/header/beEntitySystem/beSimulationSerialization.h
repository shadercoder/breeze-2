/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SIMULATIONSERIALIZATION
#define BE_ENTITYSYSTEM_SIMULATIONSERIALIZATION

#include "beEntitySystem.h"
#include "beSerialization.h"

namespace beEntitySystem
{

class Simulation;

/// Simulation serialization.
typedef Serialization<Simulation> SimulationSerialization;

/// Gets the simulation serialization register.
BE_ENTITYSYSTEM_API SimulationSerialization& GetSimulationSerialization();

}

#endif