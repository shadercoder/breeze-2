/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_CONTROLLERDRIVENSERIALIZER
#define BE_ENTITYSYSTEM_CONTROLLERDRIVENSERIALIZER

#include "beEntitySystem.h"
#include "beSerializer.h"

namespace beEntitySystem
{

class ControllerDriven;

/// Controller-driven serializer.
template <class Serializable>
class ControllerDrivenSerializer : public Serializer<Serializable>
{
public:
	/// Constructor.
	BE_ENTITYSYSTEM_API ControllerDrivenSerializer(const utf8_ntri &type);
	/// Destructor.
	BE_ENTITYSYSTEM_API ~ControllerDrivenSerializer();

	/// Loads all controllers from the given xml node.
	BE_ENTITYSYSTEM_API static void LoadControllers(ControllerDriven *pSerializable, const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue);
	/// Saves all controllers of the given serializable object to the given XML node.
	BE_ENTITYSYSTEM_API static void SaveControllers(const ControllerDriven *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue);

	/// Loads a serializable object from the given xml node.
	BE_ENTITYSYSTEM_API virtual void Load(Serializable *pSerializable, const rapidxml::xml_node<lean::utf8_t> &node, 
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
	/// Saves the given serializable object to the given XML node.
	BE_ENTITYSYSTEM_API virtual void Save(const Serializable *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const;
};

}

#endif