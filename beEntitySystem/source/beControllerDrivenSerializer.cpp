/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beControllerDrivenSerializer.h"

#include "beEntitySystem/beController.h"
#include "beEntitySystem/beControllerDriven.h"
#include "beEntitySystem/beControllerSerialization.h"
#include "beEntitySystem/beControllerSerializer.h"

#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Constructor.
template <class Serializable>
ControllerDrivenSerializer<Serializable>::ControllerDrivenSerializer(const utf8_ntri &type)
	: Serializer<Serializable>(type)
{
}

// Destructor.
template <class Serializable>
ControllerDrivenSerializer<Serializable>::~ControllerDrivenSerializer()
{
}

// Loads a serializable object from the given xml node.
template <class Serializable>
void ControllerDrivenSerializer<Serializable>::Load(Serializable *pSerializable, const rapidxml::xml_node<lean::utf8_t> &node, 
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const
{
	Serializer<Serializable>::Load(pSerializable, node, parameters, queue);
	LoadControllers(pSerializable, node, parameters, queue);
}

// Saves the given entity to the given XML node.
template <class Serializable>
void ControllerDrivenSerializer<Serializable>::Save(const Serializable *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const
{
	Serializer<Serializable>::Save(pSerializable, node, parameters, queue);
	SaveControllers(pSerializable, node, parameters, queue);
}

// Loads all controllers from the given xml node.
template <class Serializable>
void ControllerDrivenSerializer<Serializable>::LoadControllers(ControllerDriven *pSerializable, const rapidxml::xml_node<lean::utf8_t> &node, 
	beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue)
{
	const ControllerSerialization &controllerSerialization = GetControllerSerialization();

	for (const rapidxml::xml_node<utf8_t> *pControllersNode = node.first_node("controllers");
		pControllersNode; pControllersNode = pControllersNode->next_sibling("controllers"))
		for (const rapidxml::xml_node<utf8_t> *pControllerNode = pControllersNode->first_node();
			pControllerNode; pControllerNode = pControllerNode->next_sibling())
		{
			lean::resource_ptr<Controller> pController = controllerSerialization.Load(*pControllerNode, parameters, queue);

			if (pController)
				pSerializable->AddController(pController); // TODO: Order?
			else
				LEAN_LOG_ERROR_CTX("ControllerSerialization::Load()", ControllerSerializer::GetName(*pControllerNode));
		}
}

// Saves all controllers of the given serializable object to the given XML node.
template <class Serializable>
void ControllerDrivenSerializer<Serializable>::SaveControllers(const ControllerDriven *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
	beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue)
{
	const typename Serializable::Controllers &controllers = pSerializable->GetControllers();

	if (!controllers.empty())
	{
		rapidxml::xml_document<utf8_t> &document = *node.document();

		rapidxml::xml_node<utf8_t> &controllersNode = *lean::allocate_node<utf8_t>(document, "controllers");
		// ORDER: Append FIRST, otherwise parent document == nullptrs
		node.append_node(&controllersNode);

		const ControllerSerialization &controllerSerialization = GetControllerSerialization();

		for (typename Serializable::Controllers::const_iterator itController = controllers.begin();
			itController != controllers.end(); itController++)
			// WARNING: may contain nullptrs
			if (*itController)
			{
				rapidxml::xml_node<utf8_t> &controllerNode = *lean::allocate_node<utf8_t>(document, "c");
				// ORDER: Append FIRST, otherwise document == nullptr
				controllersNode.append_node(&controllerNode);

				controllerSerialization.Save(*itController, controllerNode, parameters, queue);
			}
	}
}

} // namespace
