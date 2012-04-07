/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_REFLECTION_PROPERTIES
#define BE_CORE_REFLECTION_PROPERTIES

#include "beCore.h"
#include "beReflectionPropertyProvider.h"
#include <lean/properties/property.h>
#include <lean/properties/property_accessors.h>
#include <lean/properties/property_types.h>
#include <lean/properties/property_collection.h>
#include "beGenericSerialization.h"

namespace beCore
{

/// Reflection property.
struct ReflectionProperty : public lean::ui_property_desc<ReflectionPropertyProvider, int2, ReflectionProperty>
{
	typedef lean::ui_property_desc<ReflectionPropertyProvider, int2, ReflectionProperty> base_type;

	uint2 typeID;		///< Serialization type.
	bool persistent;	///< Persistent property.

	/// Constructs an empty property description.
	ReflectionProperty() { }
	/// Constructs a property description from the given parameters.
	ReflectionProperty(const utf8_ntri &name, uint4 typeID, const lean::property_type_info &type, size_t count, int2 widget, bool bPersistent)
		: base_type(name, type, count, widget),
		// MONITOR: Type ID truncated to two bytes.
		typeID(static_cast<uint2>(typeID)),
		persistent(bPersistent) { }
};

/// Constructs a property description from the given parameters.
template <class Type>
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int2 widget, size_t count, bool bPersistent = true)
{
	return ReflectionProperty(name, RegisterType<Type>(), lean::get_property_type_info<Type>(), count, widget, bPersistent);
}

/// Constructs a property description from the given parameters.
template <class Type, size_t Count>
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int2 widget, bool bPersistent = true)
{
	return ReflectionProperty(name, RegisterType<Type>(), lean::get_property_type_info<Type>(), Count, widget, bPersistent);
}

namespace Impl
{
	template <class Type>
	struct DeducePropertyElements
	{
		typedef Type type;
		static const size_t count = 1;
	};
	template <class Type, size_t Count>
	struct DeducePropertyElements<Type[Count]>
	{
		typedef Type type;
		static const size_t count = Count;
	};
}

/// Constructs a property description from the given parameters.
template <class Type>
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int2 widget, bool bPersistent = true)
{
	return ReflectionProperty(name,
			RegisterType< typename Impl::DeducePropertyElements<Type>::type >(),
			lean::get_property_type_info< typename Impl::DeducePropertyElements<Type>::type >(),
			typename Impl::DeducePropertyElements<Type>::count, widget,
			bPersistent
		);
}

/// Property collection.
typedef lean::property_collection<beCore::ReflectionPropertyProvider, ReflectionProperty> ReflectionProperties;

} // namespace

/// @addtogroup GlobalMacros
/// @{

/// Constructs a property getter that provides access to the given number of the given values.
#define BE_CORE_PROPERTY_CONSTANT(constants, count) ::lean::properties::make_property_constant<ReflectionPropertyProvider>(constants, count)

/// Constructs a property setter that provides access to object values using the given setter method.
#define BE_CORE_PROPERTY_SETTER(setter) ::lean::properties::deduce_accessor_binder(setter) \
	.set_base<::beCore::ReflectionPropertyProvider>().bind_setter<setter>()
/// Constructs a property setter that provides access to object values using the given getter method.
#define BE_CORE_PROPERTY_GETTER(getter) ::lean::properties::deduce_accessor_binder(getter) \
	.set_base<::beCore::ReflectionPropertyProvider>().bind_getter<getter>()

/// Constructs a property setter that provides access to object values using the given setter method, splitting or merging values of the given type to values of the setter parameter type.
#define BE_CORE_PROPERTY_SETTER_UNION(setter, value_type) ::lean::properties::deduce_accessor_binder(setter) \
	.set_base<::beCore::ReflectionPropertyProvider>().set_value<value_type>().bind_setter<setter>()
/// Constructs a property getter that provides access to object values using the given getter method, splitting or merging values of the given type to values of the getter parameter (return) type.
#define BE_CORE_PROPERTY_GETTER_UNION(getter, value_type) ::lean::properties::deduce_accessor_binder(getter) \
	.set_base<::beCore::ReflectionPropertyProvider>().set_value<value_type>().bind_getter<getter>()

/// @}

#endif