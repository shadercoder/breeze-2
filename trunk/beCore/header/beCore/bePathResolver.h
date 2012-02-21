/******************************************************/
/* breeze Engine Core Module     (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_CORE_PATH_RESOLVER
#define BE_CORE_PATH_RESOLVER

#include "beCore.h"
#include "beExchangeContainers.h"
#include <lean/smart/cloneable.h>

namespace beCore
{

/// Path resolver interface.
class PathResolver : public lean::cloneable
{
protected:
	LEAN_INLINE PathResolver& operator =(const PathResolver&) { return *this; }
	LEAN_INLINE ~PathResolver() throw() { }

public:
	/// Resolves the given file name.
	virtual Exchange::utf8_string Resolve(const utf8_ntri &file, bool bThrow = false) const = 0;
};

} // namespace

#endif