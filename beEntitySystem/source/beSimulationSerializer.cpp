/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beSimulationSerializer.h"
#include "beEntitySystem/beSimulation.h"

#include "beEntitySystem/beSimulationSerialization.h"

#include "beEntitySystem/beControllerSerialization.h"
#include "beEntitySystem/beControllerSerializer.h"

namespace beEntitySystem
{
	// Instantiate simulation serializer
	template class Serializer<Simulation>;
	template class ControllerDrivenSerializer<Simulation>;
}
// Link simulation serialization
#include "beSerializer.cpp"
#include "beControllerDrivenSerializer.cpp"

namespace beEntitySystem
{
	
// Constructor.
SimulationSerializer::SimulationSerializer(const utf8_ntri &type)
	: ControllerDrivenSerializer<Simulation>(type)
{
}

// Destructor.
SimulationSerializer::~SimulationSerializer()
{
}

/// Creates a simulation object from the given parameters.
lean::resource_ptr<Simulation, true> SimulationSerializer::Create(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const
{
	// TODO
	return nullptr;
}

// Loads a simulation from the given xml node.
lean::resource_ptr<Simulation, true> SimulationSerializer::Load(const rapidxml::xml_node<lean::utf8_t> &node, 
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
	lean::resource_ptr<Simulation> pSimulation = new Simulation( GetName(node) );
	ControllerDrivenSerializer<Simulation>::Load(pSimulation, node, parameters, queue);
	return pSimulation.transfer();
}

namespace
{

// Plugin class.
struct SimulationSerialization
{
	SimulationSerializer Serializer;

	SimulationSerialization()
		: Serializer( Simulation::GetSimulationType() )
	{
		GetSimulationSerialization().AddSerializer(&Serializer);
	}
	~SimulationSerialization()
	{
		GetSimulationSerialization().RemoveSerializer(&Serializer);
	}
};

const SimulationSerialization SimulationSerialization;

} // namespace

} // namespace
