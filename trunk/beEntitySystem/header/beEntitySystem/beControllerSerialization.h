/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_CONTROLLERSERIALIZATION
#define BE_ENTITYSYSTEM_CONTROLLERSERIALIZATION

#include "beEntitySystem.h"
#include "beSerialization.h"

namespace beEntitySystem
{

class Controller;
class ControllerSerializer;

/// Controller serialization.
typedef Serialization<Controller, ControllerSerializer> ControllerSerialization;

/// Gets the controller serialization register.
BE_ENTITYSYSTEM_API ControllerSerialization& GetControllerSerialization();

/// Instantiate this to add a serializer of the given type.
template <class ControllerSerializer>
struct ControllerSerializationPlugin
{
	/// Serializer.
	ControllerSerializer Serializer;

	/// Adds the serializer.
	ControllerSerializationPlugin()
	{
		GetControllerSerialization().AddSerializer(&Serializer);
	}
	/// Removes the serializer.
	~ControllerSerializationPlugin()
	{
		GetControllerSerialization().RemoveSerializer(&Serializer);
	}
};

}

#endif