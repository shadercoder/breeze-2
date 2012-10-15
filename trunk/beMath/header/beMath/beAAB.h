/*****************************************************/
/* breeze Engine Math Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_MATH_AAB
#define BE_MATH_AAB

#include "beMath.h"
#include "beAABDef.h"
#include "beVector.h"
#include "beMatrix.h"

namespace beMath
{

/// Transforms the given aab.
template <class Component, size_t Dimension>
inline aab<Component, Dimension> mul(const aab<Component, Dimension> &box, const matrix<Component, Dimension, Dimension> &mat)
{
	using std::swap;

	aab<Component, Dimension> result;

	for (size_t i = 0; i < Dimension; ++i)
		for (size_t j = 0; j < Dimension; ++j)
		{
			Component e = mat[i][j];

			Component a = box.min[i] * e;
			Component b = box.max[i] * e;

			if (e < Component(0))
				swap(a, b);

			result.min[j] += a;
			result.max[j] += b;
		}

	return result;
}

/// Transforms the given aab.
template <class Component, size_t Dimension>
inline aab<Component, Dimension> mulh(const aab<Component, Dimension> &box, const matrix<Component, Dimension + 1, Dimension + 1> &mat)
{
	using std::swap;

	aab<Component, Dimension> result( mat[Dimension], mat[Dimension] );

	for (size_t i = 0; i < Dimension; ++i)
		for (size_t j = 0; j < Dimension; ++j)
		{
			Component e = mat[i][j];

			Component a = box.min[i] * e;
			Component b = box.max[i] * e;

			if (e < Component(0))
				swap(a, b);

			result.min[j] += a;
			result.max[j] += b;
		}

	return result;
}

} // namespace

#endif