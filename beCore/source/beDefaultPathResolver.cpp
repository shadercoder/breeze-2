/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/beDefaultPathResolver.h"

#include <lean/io/filesystem.h>

namespace beCore
{

// Resolves the given file name.
Exchange::utf8_string DefaultPathResolver::Resolve(const utf8_ntri &file, bool bThrow) const
{
	return lean::absolute_path<Exchange::utf8_string>(file);
}

/// Constructs and returns a clone of this path resolver.
DefaultPathResolver* DefaultPathResolver::clone() const
{
	return new DefaultPathResolver(*this);
}
/// Destroys an include manager.
void DefaultPathResolver::destroy() const
{
	delete this;
}

} // namespace