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

/// Widget enumeration.
namespace Widget
{
	/// Enumeration.
	enum T
	{
		None,			///< No widget.
		Raw,			///< Raw value widget.
		Slider,			///< Slider widget.
		Color,			///< Color widget.
		Angle,			///< Angle widget.
		Orientation		///< Orientation widget.
	};
}

/// Reflection property.
struct ReflectionProperty : public lean::ui_property_desc<ReflectionPropertyProvider, int, ReflectionProperty>
{
	typedef lean::ui_property_desc<ReflectionPropertyProvider, int, ReflectionProperty> base_type;

	uint4 typeID;	///< Serialization type.

	/// Constructs an empty property description.
	ReflectionProperty() { }
	/// Constructs a property description from the given parameters.
	ReflectionProperty(const utf8_ntri &name, uint4 typeID, const lean::property_type &type, size_t count, int widget)
		: base_type(name, type, count, widget),
		typeID(typeID) { }
};

/// Constructs a property description from the given parameters.
template <class Type>
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int widget, size_t count)
{
	return ReflectionProperty(name, RegisterType<Type>(), lean::get_property_type<Type>(), count, widget);
}

/// Constructs a property description from the given parameters.
template <class Type, size_t Count>
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int widget)
{
	return ReflectionProperty(name, RegisterType<Type>(), lean::get_property_type<Type>(), Count, widget);
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
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int widget)
{
	return ReflectionProperty(name,
		RegisterType< typename Impl::DeducePropertyElements<Type>::type >(),
		lean::get_property_type< typename Impl::DeducePropertyElements<Type>::type >(),
		typename Impl::DeducePropertyElements<Type>::count, widget);
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