/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_GENERIC_TEXT_SERIALIZATION
#define BE_CORE_GENERIC_TEXT_SERIALIZATION

#include "beCore.h"
#include "beTextSerialization.h"
#include "beGenericTextSerializer.h"

namespace beCore
{

/// Registers a type using generic serialization.
template <class Type>
LEAN_NOINLINE uint4 RegisterType(TextSerialization &serialization)
{
	uint4 typeID = serialization.GetTypeIndex()->GetID(typeid(Type).name());
	
	if (!serialization.GetSerializer(typeID))
	{
		static const GenericTextSerializer< lean::io::generic_serialization<Type> > textSerializer;
		serialization.AddSerializer(&textSerializer);
	}

	return typeID;
}

/// Registers a type using integer serialization.
template <class Type>
LEAN_NOINLINE uint4 RegisterIntType(TextSerialization &serialization)
{
	uint4 typeID = serialization.GetTypeIndex()->GetID(typeid(Type).name());
	
	if (!serialization.GetSerializer(typeID))
	{
		static const GenericTextSerializer< lean::io::int_serialization<Type> > textSerializer;
		serialization.AddSerializer(&textSerializer);
	}

	return typeID;
}

template <>
LEAN_INLINE uint4 RegisterType<unsigned char>(TextSerialization &serialization) { return RegisterIntType<unsigned char>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<char>(TextSerialization &serialization) { return RegisterIntType<char>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<unsigned short>(TextSerialization &serialization) { return RegisterIntType<unsigned short>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<short>(TextSerialization &serialization) { return RegisterIntType<short>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<unsigned int>(TextSerialization &serialization) { return RegisterIntType<unsigned int>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<int>(TextSerialization &serialization) { return RegisterIntType<int>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<unsigned long>(TextSerialization &serialization) { return RegisterIntType<unsigned long>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<long>(TextSerialization &serialization) { return RegisterIntType<long>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<unsigned long long>(TextSerialization &serialization) { return RegisterIntType<unsigned long long>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<long long>(TextSerialization &serialization) { return RegisterIntType<long long>(serialization); }

/// Registers a type using float serialization.
template <class Type>
LEAN_NOINLINE uint4 RegisterFloatType(TextSerialization &serialization)
{
	uint4 typeID = serialization.GetTypeIndex()->GetID(typeid(Type).name());
	
	if (!serialization.GetSerializer(typeID))
	{
		static const GenericTextSerializer< lean::io::float_serialization<Type> > textSerializer;
		serialization.AddSerializer(&textSerializer);
	}

	return typeID;
}

template <>
LEAN_INLINE uint4 RegisterType<float>(TextSerialization &serialization) { return RegisterFloatType<float>(serialization); }
template <>
LEAN_INLINE uint4 RegisterType<double>(TextSerialization &serialization) { return RegisterFloatType<double>(serialization); }

/// Unregisters a type.
template <class Type>
LEAN_NOINLINE void UnregisterType(TextSerialization &serialization = GetTextSerialization())
{
	serialization.RemoveSerializer(typeid(Type).name());
}

} // namespace

#endif