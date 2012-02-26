/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/beComponentTypes.h"
#include "beCore/beComponentReflector.h"

namespace beCore
{

// Constructor.
ComponentTypes::ComponentTypes()
{
}

// Destructor.
ComponentTypes::~ComponentTypes()
{
}

// Adds the given component reflector & corresponding component type.
void ComponentTypes::AddComponentType(const ComponentReflector *pReflector)
{
	m_reflectors[pReflector->GetType().to<utf8_string>()] = pReflector;
}

// Removes the given component reflector & corresponding component type.
bool ComponentTypes::RemoveComponentType(const ComponentReflector *pReflector)
{
	return (m_reflectors.erase(pReflector->GetType().to<utf8_string>()) != 0);
}

// Gets a component reflector for the given component type.
const ComponentReflector* ComponentTypes::GetReflector(const utf8_ntri &name) const
{
	reflector_map::const_iterator it = m_reflectors.find( name.to<utf8_string>() );

	return (it != m_reflectors.end())
		? it->second
		: nullptr;
}

// Gets the number of reflectors.
uint4 ComponentTypes::GetReflectorCount() const
{
	return static_cast<uint4>(m_reflectors.size());
}

// Gets all reflectors.
void ComponentTypes::GetReflectors(const ComponentReflector **reflectors) const
{
	for (reflector_map::const_iterator it = m_reflectors.begin(); it != m_reflectors.end(); ++it)
		*reflectors++ = it->second;
}

// Gets the component type register.
ComponentTypes& GetComponentTypes()
{
	static ComponentTypes componentTypes;
	return componentTypes;
}

} // namespace
