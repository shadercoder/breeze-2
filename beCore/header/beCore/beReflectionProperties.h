/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_CORE_REFLECTION_PROPERTIES
#define BE_CORE_REFLECTION_PROPERTIES

#include "beCore.h"
#include "beReflectionPropertyProvider.h"
#include <lean/properties/property.h>
#include <lean/properties/property_accessors.h>
#include "beValueType.h"
#include <lean/properties/property_collection.h>
#include "beBuiltinTypes.h"

namespace beCore
{

struct PropertyPersistence
{
	enum T
	{
		None = 0x0,
		Read = 0x1,
		Write = 0x2,
		ReadWrite = Read | Write
	};
};

struct ValueTypeDesc;

/// Reflection property.
struct ReflectionProperty : public lean::ui_property_desc<ReflectionPropertyProvider, int2, ReflectionProperty, ValueTypeDesc>
{
	uint2 persistence;				///< Persistent property.

	/// Constructs an empty property description.
	ReflectionProperty() { }
	/// Constructs a property description from the given parameters.
	ReflectionProperty(const utf8_ntri &name, const ValueTypeDesc &typeDesc, size_t count, int2 widget, uint2 persistence)
		: ui_property_desc(name, typeDesc, count, widget),
		persistence(persistence) { }
};

/// Constructs a property description from the given parameters.
template <class Type>
LEAN_INLINE ReflectionProperty MakeReflectionPropertyN(const utf8_ntri &name, int2 widget, size_t count, uint2 persistence = PropertyPersistence::ReadWrite)
{
	return ReflectionProperty(name, GetBuiltinType<Type>(), count, widget, persistence);
}

/// Constructs a property description from the given parameters.
template <class Type, size_t Count>
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int2 widget, uint2 persistence = PropertyPersistence::ReadWrite)
{
	return ReflectionProperty(name, GetBuiltinType<Type>(), Count, widget, persistence);
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
LEAN_INLINE ReflectionProperty MakeReflectionProperty(const utf8_ntri &name, int2 widget, uint2 persistence = PropertyPersistence::ReadWrite)
{
	return ReflectionProperty(name,
			GetBuiltinType< typename Impl::DeducePropertyElements<Type>::type >(),
			typename Impl::DeducePropertyElements<Type>::count, widget,
			persistence
		);
}

/// Gets the reflection property range.
template <size_t Count>
LEAN_INLINE lean::range<const ReflectionProperty*> ToPropertyRange(const ReflectionProperty (&properties)[Count])
{
	return lean::make_range(&properties[0], &properties[Count]);
}

/// Property collection.
typedef lean::property_collection<beCore::ReflectionPropertyProvider, ReflectionProperty> ReflectionProperties;
/// Gets the reflection property range.
LEAN_INLINE lean::range<const ReflectionProperty*> ToPropertyRange(const ReflectionProperties &properties)
{
	return lean::make_range(properties.data(), properties.data_end());
}

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

/// Associates the given properties with the given type.
#define BE_CORE_ASSOCIATE_PROPERTIES_X(type, dynamic_method, static_method, properties) \
	lean::range<const beCore::ReflectionProperty*> type::static_method() { return ToPropertyRange(properties); } \
	lean::range<const beCore::ReflectionProperty*> type::dynamic_method() const { return ToPropertyRange(properties); }
/// Associates the given properties with the given type.
#define BE_CORE_ASSOCIATE_PROPERTIES(type, properties) \
	BE_CORE_ASSOCIATE_PROPERTIES_X(type, GetReflectionProperties, GetOwnProperties, properties)
	

/// @}

#endif