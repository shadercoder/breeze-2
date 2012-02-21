/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/beTypeIndex.h"

namespace beCore
{

// Gets the global type index.
TypeIndex& GetTypeIndex()
{
	static beCore::Identifiers types;
	return types;
}

} // namespace
