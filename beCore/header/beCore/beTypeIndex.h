/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_TYPE_INDEX
#define BE_CORE_TYPE_INDEX

#include "beCore.h"
#include "beIdentifiers.h"

namespace beCore
{

/// Type index.
typedef beCore::Identifiers TypeIndex;
/// Gets the global type index.
BE_CORE_API TypeIndex& GetTypeIndex();

} // namespace

#endif