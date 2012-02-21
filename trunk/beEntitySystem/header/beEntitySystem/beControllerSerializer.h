/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_CONTROLLERSERIALIZER
#define BE_ENTITYSYSTEM_CONTROLLERSERIALIZER

#include "beEntitySystem.h"
#include "beSerializer.h"

namespace beEntitySystem
{

class Controller;

/// Controller serializer.
class ControllerSerializer : public Serializer<Controller>
{
public:
	/// Constructor.
	BE_ENTITYSYSTEM_API ControllerSerializer(const utf8_ntri &type);
	/// Destructor.
	BE_ENTITYSYSTEM_API ~ControllerSerializer();

	// Fix overloading
	using Serializer<Controller>::Load;
	/// Loads a controller from the given xml node.
	BE_ENTITYSYSTEM_API virtual void Load(Controller *pController, const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
	/// Saves the given controller to the given XML node.
	BE_ENTITYSYSTEM_API virtual void Save(const Controller *pController, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const;
};

}

#endif