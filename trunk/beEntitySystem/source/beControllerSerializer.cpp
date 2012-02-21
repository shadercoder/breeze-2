/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beControllerSerializer.h"
#include "beEntitySystem/beController.h"

#include <beCore/bePropertySerialization.h>

namespace beEntitySystem
{
	// Instantiate controller serializer
	template class Serializer<Controller>;
}
// Link controller serialization
#include "beSerializer.cpp"

namespace beEntitySystem
{

// Constructor.
ControllerSerializer::ControllerSerializer(const utf8_ntri &type)
	: Serializer<Controller>(type)
{
}

// Destructor.
ControllerSerializer::~ControllerSerializer()
{
}

// Loads a controller from the given xml node.
void ControllerSerializer::Load(Controller *pController, const rapidxml::xml_node<lean::utf8_t> &node, beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
	// Properties
	LoadProperties(*pController, node);

	Serializer<Controller>::Load(pController, node, parameters, queue);
}

// Saves the given controller to the given XML node.
void ControllerSerializer::Save(const Controller *pController, rapidxml::xml_node<lean::utf8_t> &node, beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const
{
	// TODO: Controller IDs!
//	SetID(pController->GetPersistentID(), node);

	// Properties
	SaveProperties(*pController, node);

	Serializer<Controller>::Save(pController, node, parameters, queue);
}

} // namespace