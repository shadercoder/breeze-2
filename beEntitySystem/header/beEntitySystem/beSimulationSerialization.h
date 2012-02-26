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

/// Instantiate this to add a serializer of the given type.
template <class SimulationSerializer>
struct SimulationSerializationPlugin
{
	/// Serializer.
	SimulationSerializer Serializer;

	/// Adds the serializer.
	SimulationSerializationPlugin()
	{
		GetSimulationSerialization().AddSerializer(&Serializer);
	}
	/// Removes the serializer.
	~SimulationSerializationPlugin()
	{
		GetSimulationSerialization().RemoveSerializer(&Serializer);
	}
};

}

#endif