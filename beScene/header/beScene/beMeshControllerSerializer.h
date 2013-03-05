/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#pragma once
#ifndef BE_SCENE_MESH_CONTROLLER_SERIALIZER
#define BE_SCENE_MESH_CONTROLLER_SERIALIZER

#include "beScene.h"
#include <beEntitySystem/beControllerSerializer.h>

namespace beScene
{

// Prototypes
class MeshController;

/// Serializes mesh controllers.
class MeshControllerSerializer : public beEntitySystem::ControllerSerializer
{
public:
	typedef beEntitySystem::ControllerSerializer ControllerSerializer;

	/// Constructor.
	BE_SCENE_API MeshControllerSerializer();
	/// Destructor.
	BE_SCENE_API ~MeshControllerSerializer();

	/// Gets a list of creation parameters.
	BE_SCENE_API virtual beCore::ComponentParameters GetCreationParameters() const;
	/// Creates a serializable object from the given parameters.
	BE_SCENE_API virtual lean::scoped_ptr<Serializable, lean::critical_ref> Create(
		const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const;

	/// Loads a mesh controller from the given xml node.
	BE_SCENE_API virtual lean::scoped_ptr<beEntitySystem::Controller, lean::critical_ref> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::LoadJob> &queue) const;
	/// Saves the given mesh controller to the given XML node.
	BE_SCENE_API virtual void Save(const beEntitySystem::Controller *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue) const;
};

} // namespace

#endif