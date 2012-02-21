/*****************************************************/
/* breeze Engine Math Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_MATH_SPHERE
#define BE_MATH_SPHERE

#include "beMath.h"
#include "beSphereDef.h"
#include "beVector.h"
#include "beMatrix.h"

namespace beMath
{

/// Transforms the given sphere.
template <class Component, size_t Dimension>
LEAN_INLINE sphere<Component, Dimension> mul(const matrix<Component, Dimension + 1, Dimension + 1> &left, const sphere<Component, Dimension> &right)
{
	// TODO: Scale
	return sphere<Component, Dimension>( mul(left, right.p()), right.r() );
}

/// Transforms the given sphere.
template <class Component, size_t Dimension>
LEAN_INLINE sphere<Component, Dimension> mul(const sphere<Component, Dimension> &left, const matrix<Component, Dimension + 1, Dimension + 1> &right)
{
	// TODO: Scale
	return sphere<Component, Dimension>( mul(left.p(), right), left.r() );
}

} // namespace

#endif