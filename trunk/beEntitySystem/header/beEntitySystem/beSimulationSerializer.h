/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SIMULATIONSERIALIZER
#define BE_ENTITYSYSTEM_SIMULATIONSERIALIZER

#include "beEntitySystem.h"
#include "beControllerDrivenSerializer.h"

namespace beEntitySystem
{

class Simulation;

/// Simulation serializer.
class SimulationSerializer : public ControllerDrivenSerializer<Simulation>
{
public:
	/// Constructor.
	BE_ENTITYSYSTEM_API SimulationSerializer(const utf8_ntri &type);
	/// Destructor.
	BE_ENTITYSYSTEM_API ~SimulationSerializer();

	/// Loads a simulation from the given xml node.
	BE_ENTITYSYSTEM_API virtual lean::resource_ptr<Simulation, true> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
};

}

#endif