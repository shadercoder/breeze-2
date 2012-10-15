/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SERIALIZER
#define BE_ENTITYSYSTEM_SERIALIZER

#include "beEntitySystem.h"
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>
#include <lean/rapidxml/rapidxml.hpp>

#include <beCore/beParameters.h>

// Prototypes
namespace beCore
{
	class ParameterSet;
}

namespace beEntitySystem
{

/// Specific parameter.
struct CreationParameter
{
	utf8_ntr Name;	///< Parameter name.
	utf8_ntr Type;	///< Parameter type.
	bool Optional;	///< True, if optional.

	/// Constructor
	CreationParameter(const utf8_ntr &name, const utf8_ntr &type, bool bOptional = false)
		: Name(name),
		Type(type),
		Optional(bOptional) { }
};

// Prototypes
class LoadJob;
class SaveJob;
template <class Job>
class SerializationQueue;

/// Serializer.
template <class Serializable>
class Serializer : public lean::noncopyable
{
private:
	utf8_string m_type;

public:
	/// Serializable type.
	typedef Serializable Serializable;
	/// Serialization parameter range.
	typedef lean::range<const CreationParameter*> SerializationParameters;

	/// Constructor.
	BE_ENTITYSYSTEM_API Serializer(const utf8_ntri &type);
	/// Destructor.
	BE_ENTITYSYSTEM_API ~Serializer();

	/// Sets the name of the serializable object stored in the given xml node.
	BE_ENTITYSYSTEM_API static void SetName(const utf8_ntri &name, rapidxml::xml_node<lean::utf8_t> &node);
	/// Gets the name of the serializable object stored in the given xml node.
	BE_ENTITYSYSTEM_API static utf8_ntr GetName(const rapidxml::xml_node<lean::utf8_t> &node);
	/// Gets the type of the serializable object stored in the given xml node.
	BE_ENTITYSYSTEM_API static utf8_ntr GetType(const rapidxml::xml_node<lean::utf8_t> &node);
	/// Gets the ID of the serializable object stored in the given xml node.
	BE_ENTITYSYSTEM_API static uint8 GetID(const rapidxml::xml_node<lean::utf8_t> &node);
	/// Sets the ID of the serializable object stored in the given xml node.
	BE_ENTITYSYSTEM_API static void SetID(uint8 id, rapidxml::xml_node<lean::utf8_t> &node);

	/// Gets a list of creation parameters.
	BE_ENTITYSYSTEM_API virtual SerializationParameters GetCreationParameters() const;
	/// Creates a serializable object from the given parameters.
	virtual lean::resource_ptr<Serializable, true> Create(const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters) const = 0;

	/// Loads a serializable object from the given xml node.
	virtual lean::resource_ptr<Serializable, true> Load(const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const = 0;
	/// Loads a serializable object from the given xml node.
	BE_ENTITYSYSTEM_API virtual void Load(Serializable *pSerializable, const rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const;
	/// Saves the given serializable object to the given XML node.
	BE_ENTITYSYSTEM_API virtual void Save(const Serializable *pSerializable, rapidxml::xml_node<lean::utf8_t> &node,
		beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const;

	/// Gets the type of objects serialized.
	LEAN_INLINE const utf8_string& GetType() const { return m_type; }
};

}

#endif