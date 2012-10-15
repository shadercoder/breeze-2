/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_RIGID_CONTROLLER_SERIALIZER
#define BE_PHYSICS_RIGID_CONTROLLER_SERIALIZER

#include "beScene.h"
#include <beEntitySystem/beControllerSerializer.h>

namespace bePhysics
{

/// Serializes rigid controllers.
template <class RigidController>
class RigidControllerSerializer : public beEntitySystem::ControllerSerializer
{
public:
	typedef beEntitySystem::ControllerSerializer ControllerSerializer;
	typedef RigidController RigidController;

	/// Constructor.
	BE_PHYSICS_API RigidControllerSerializer();
	/// Destructor.
	BE_PHYSICS_API ~RigidControllerSerializer();

	/// Gets a list of creation parameters.
	BE_PHYSICS_API virtual SerializationParameters GetCreationParameters() const;
	/// Creates a serializable object from the given parameters.
	BE_PHYSICS_API virtual lean::resource_ptr<Serializable, true> Create(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const;

	/// Loads a mesh controller from the given xml node.
	BE_PHYSICS_API virtual lean::resource_ptr<beEntitySystem::Controller, true> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::LoadJob> &queue) const;
	/// Saves the given mesh controller to the given XML node.
	BE_PHYSICS_API virtual void Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::SaveJob> &queue) const;
};

class RigidStaticController;
class RigidDynamicController;

/// Rigid static controller serializer class.
typedef RigidControllerSerializer<RigidStaticController> RigidStaticControllerSerializer;
/// Rigid dynamic controller serializer class.
typedef RigidControllerSerializer<RigidDynamicController> RigidDynamicControllerSerializer;

} // namespace

#endif