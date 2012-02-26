/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_COMPONENT_TYPES
#define BE_CORE_COMPONENT_TYPES

#include "beCore.h"
#include <lean/tags/noncopyable.h>
#include <unordered_map>

namespace beCore
{

class ComponentReflector;

/// Manages component types & reflectors.
class ComponentTypes : public lean::noncopyable
{
private:
	typedef std::unordered_map<utf8_string, const ComponentReflector*> reflector_map;
	reflector_map m_reflectors;

public:
	/// Constructor.
	BE_CORE_API ComponentTypes();
	/// Destructor.
	BE_CORE_API ~ComponentTypes();

	/// Adds the given component reflector & corresponding component type.
	BE_CORE_API void AddComponentType(const ComponentReflector *pReflector);
	/// Removes the given component reflector & corresponding component type.
	BE_CORE_API bool RemoveComponentType(const ComponentReflector *pReflector);

	/// Gets a component reflector for the given component type.
	BE_CORE_API const ComponentReflector* GetReflector(const utf8_ntri &name) const;

	/// Gets the number of reflectors.
	BE_CORE_API uint4 GetReflectorCount() const;
	/// Gets all reflectors.
	BE_CORE_API void GetReflectors(const ComponentReflector **reflectors) const;
};

/// Gets the component type register.
BE_CORE_API ComponentTypes& GetComponentTypes();

/// Instantiate this to add a reflector of the given type.
template <class ComponentReflector>
struct ComponentTypePlugin
{
	/// Reflector.
	ComponentReflector Reflector;

	/// Adds the reflector.
	ComponentTypePlugin()
	{
		GetComponentTypes().AddComponentType(&Reflector);
	}
	/// Removes the reflector.
	~ComponentTypePlugin()
	{
		GetComponentTypes().RemoveComponentType(&Reflector);
	}
};

} // namespace

#endif