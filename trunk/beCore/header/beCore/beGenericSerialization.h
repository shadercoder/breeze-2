/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_GENERIC_SERIALIZATION
#define BE_CORE_GENERIC_SERIALIZATION

#include "beCore.h"
#include "beGenericTextSerialization.h"

namespace beCore
{

/// Registers a type using generic serialization.
template <class Type>
LEAN_INLINE uint4 RegisterType()
{
	return RegisterType<Type>(GetTextSerialization());
}

/// Unregisters a type.
template <class Type>
LEAN_INLINE void UnregisterType()
{
	UnregisterType<Type>(GetTextSerialization());
}

} // namespace

#endif