#ifndef MATH_H
#define MATH_H

#include "breezEd.h"

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

#include <QtCore/QPoint>

inline beMath::fvec2 toQt(const QPointF &pos)
{
	return beMath::vec( (float) pos.x(), (float) pos.y() );
}

/// Gets the camera ray dir from the given relative position.
inline void camRayDirUnderCursor(beMath::fvec3 &rayOrigin, beMath::fvec3 &rayDir,
	const beMath::fvec2 &relativePos, const beMath::fmat4 &viewProj)
{
	using namespace beMath;

	fmat4 viewProjInv = inverse(viewProj);
	
	fvec4 rayPoint1 = vec(2.0f * relativePos[0] - 1.0f, -2.0f * relativePos[1] + 1.0f, -1.0f, 1.0f);
	fvec4 rayPoint2 = vec(rayPoint1[0], rayPoint1[1], 0.0f, 1.0f);

	rayPoint1 = mul(rayPoint1, viewProjInv);
	rayPoint2 = mul(rayPoint2, viewProjInv);
	
	rayOrigin = fvec3(rayPoint1) / rayPoint1[3];
	rayDir = fvec3(rayPoint2) / rayPoint2[3] - rayOrigin;
}

#endif
