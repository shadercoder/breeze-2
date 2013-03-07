/*****************************************************/
/* breeze Engine Math Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_MATH_INTERSECT
#define BE_MATH_INTERSECT

#include "beMath.h"
#include "beVector.h"
#include "bePlane.h"
#include "beAAB.h"

namespace beMath
{

/// Computes the signed distance of the given box from the given plane.
template <class Component, size_t Dimension>
inline Component sdist(const plane<Component, Dimension> &p, const aab<Component, Dimension> &box)
{
	Component minDist = Component(0), maxDist = Component(0);

	for (size_t i = 0; i < Dimension; ++i)
		if(p[i] >= 0.0f)
		{
			minDist += p[i] * box.min[i];
			maxDist += p[i] * box.max[i];
		}
		else
		{
			minDist += p[i] * box.max[i];
			maxDist += p[i] * box.min[i];
		}

	if (minDist > p.d())
		return minDist - p.d();
	else if (maxDist < p.d())
		return maxDist - p.d();
	else
		return 0.0f;
}

/// Computes the distance of the given box from the given plane.
template <class Component, size_t Dimension>
LEAN_INLINE Component dist(const plane<Component, Dimension> &p, const aab<Component, Dimension> &box)
{
	return abs( sdist(p, box) );
}

} // namespace

#endif