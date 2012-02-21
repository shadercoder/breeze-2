/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntity.h"

#include <beCore/beReflectionProperties.h>
#include <beCore/bePersistentIDs.h>

namespace beEntitySystem
{

const beCore::ReflectionProperties EntityProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<int[3]>("cell", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&Entity::SetCell, int) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&Entity::GetCell, int) )
	<< beCore::MakeReflectionProperty<float[3]>("position", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&Entity::SetPosition, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&Entity::GetPosition, float) )
	<< beCore::MakeReflectionProperty<float[9]>("orientation", beCore::Widget::Orientation)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&Entity::SetOrientation, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&Entity::GetOrientation, float) )
	<< beCore::MakeReflectionProperty<float[3]>("scaling", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&Entity::SetScaling, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&Entity::GetScaling, float) )
	<< beCore::MakeReflectionProperty<uint8>("id", beCore::Widget::Raw)
		.set_getter( BE_CORE_PROPERTY_GETTER(&Entity::GetPersistentID) );

// Constructor.
Entity::Entity(const utf8_ntri &name)
	: m_name(name.to<utf8_string>()),
	m_orientation(beMath::fmat3::identity),
	m_scaling(1.0f),
	m_id(InvalidID),
	m_persistentID(beCore::PersistentIDs::InvalidID)
{
}

// Destructor.
Entity::~Entity()
{
}

// Sets the name.
void Entity::SetName(const utf8_ntri &name)
{
	m_name.assign(name.begin(), name.end());
}

// Gets the entity type.
utf8_ntr Entity::GetEntityType()
{
	return utf8_ntr("Entity");
}

// Gets the reflection properties.
Entity::Properties Entity::GetEntityProperties()
{
	return Properties(EntityProperties.data(), EntityProperties.data_end());
}

// Gets the reflection properties.
Entity::Properties Entity::GetReflectionProperties() const
{
	return Properties(EntityProperties.data(), EntityProperties.data_end());
}

} // namespace
