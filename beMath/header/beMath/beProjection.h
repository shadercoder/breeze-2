/*****************************************************/
/* breeze Engine Math Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_MATH_PROJECTION
#define BE_MATH_PROJECTION

#include "beMath.h"
#include "beMatrix.h"
#include "bePlane.h"

namespace beMath
{

/// Extracts the left frustum plane from the given view-projection matrix.
template <class Component>
LEAN_INLINE plane<Component, 3> frustum_left(const matrix<Component, 4, 4> &viewProj)
{
	return plane<Component, 3>(
			vec(-(viewProj[0][3] + viewProj[0][0]),
				-(viewProj[1][3] + viewProj[1][0]),
				-(viewProj[2][3] + viewProj[2][0]) ),
			viewProj[3][3] + viewProj[3][0]
		);
}

/// Extracts the right frustum plane from the given view-projection matrix.
template <class Component>
LEAN_INLINE plane<Component, 3> frustum_right(const matrix<Component, 4, 4> &viewProj)
{
	return plane<Component, 3>(
			vec(-(viewProj[0][3] - viewProj[0][0]),
				-(viewProj[1][3] - viewProj[1][0]),
				-(viewProj[2][3] - viewProj[2][0]) ),
			viewProj[3][3] - viewProj[3][0]
		);
}

/// Extracts the top frustum plane from the given view-projection matrix.
template <class Component>
LEAN_INLINE plane<Component, 3> frustum_top(const matrix<Component, 4, 4> &viewProj)
{
	return plane<Component, 3>(
			vec(-(viewProj[0][3] - viewProj[0][1]),
				-(viewProj[1][3] - viewProj[1][1]),
				-(viewProj[2][3] - viewProj[2][1]) ),
			viewProj[3][3] - viewProj[3][1]
		);
}

/// Extracts the bottom frustum plane from the given view-projection matrix.
template <class Component>
LEAN_INLINE plane<Component, 3> frustum_bottom(const matrix<Component, 4, 4> &viewProj)
{
	return plane<Component, 3>(
			vec(-(viewProj[0][3] + viewProj[0][1]),
				-(viewProj[1][3] + viewProj[1][1]),
				-(viewProj[2][3] + viewProj[2][1]) ),
			viewProj[3][3] + viewProj[3][1]
		);
}

/// Extracts the near frustum plane from the given view-projection matrix.
template <class Component>
LEAN_INLINE plane<Component, 3> frustum_near(const matrix<Component, 4, 4> &viewProj)
{
	return plane<Component, 3>(
			vec(-viewProj[0][2],
				-viewProj[1][2],
				-viewProj[2][2] ),
			viewProj[3][2]
		);
}

/// Extracts the far frustum plane from the given view-projection matrix.
template <class Component>
LEAN_INLINE plane<Component, 3> frustum_far(const matrix<Component, 4, 4> &viewProj)
{
	return plane<Component, 3>(
			vec(-(viewProj[0][3] - viewProj[0][2]),
				-(viewProj[1][3] - viewProj[1][2]),
				-(viewProj[2][3] - viewProj[2][2]) ),
			viewProj[3][3] - viewProj[3][2]
		);
}

/// Extracts the six view frustum planes from the given view-projection matrix.
template <class Component>
void extract_frustum(const matrix<Component, 4, 4> &viewProj, plane<Component, 3> *planes)
{
	planes[0] = normalize( frustum_near(viewProj) );
	planes[1] = normalize( frustum_left(viewProj) );
	planes[2] = normalize( frustum_right(viewProj) );
	planes[3] = normalize( frustum_far(viewProj) );
	planes[4] = normalize( frustum_bottom(viewProj) );
	planes[5] = normalize( frustum_top(viewProj) );
}

/*
// Computes a camera ray from the given screen coordinates
beMath::CRay beMath::ComputeScreenRay(float fX, float fY, const CMatrix &mView, const CMatrix &mProjection)
{
	CRay r;

	// Compute inverse of orthogonal camera matrix
	CMatrix mViewInverse = Reverse(mView);

	// Set origin to camera position
	r.o.x = mViewInverse[3][0];
	r.o.y = mViewInverse[3][1];
	r.o.z = mViewInverse[3][2];

	// Construct normal in view space
	r.n()[0] = (fX * 2.0f - 1.0f) / mProjection[0][0];
	r.n()[1] = -(fY * 2.0f - 1.0f) / mProjection[1][1];
	r.n()[2] = 1.0f;

	// Transform normal to world space
	r.n = Normalize( Transform3(r.n, mViewInverse) );

	return r;
}
*/

/*
/// Computes a projection matrix that contains the given plane as near plane
beMath::CMatrix beMath::ObliqueProjectionMatrix(const CMatrix &mView, const CMatrix &mProjection, const CPlane &clipPlane)
{
	CMatrix mViewInverseTranspose = Transpose(Reverse(mView));

	// Transform plane to view space
	CVector4 vViewClipPlane = -clipPlane * mViewInverseTranspose;

	// Check clip plane orientation
	if(vViewClipPlane.w > -Tolerance)
		return mProjection;

	// Compute view-space corner point
	CVector4 vFarCorner = CVector4(
		Sign0(vViewClipPlane.x + mProjection[2][0]) / mProjection[0][0],
		Sign0(vViewClipPlane.y + mProjection[2][1]) / mProjection[1][1],
		-1.0f,
		(1.0f + mProjection[2][2]) / mProjection[3][2] );
	
	// Scale clip plane
	CVector4 vProjClipPlane = vViewClipPlane * (1.0f / Dot(vViewClipPlane, vFarCorner));

	// Modify projection matrix
	return CMatrix(mProjection[0][0], mProjection[0][1], vProjClipPlane.x, mProjection[0][3],
		mProjection[1][0], mProjection[1][1], vProjClipPlane.y, mProjection[1][3],
		mProjection[2][0], mProjection[2][1], vProjClipPlane.z, mProjection[2][3],
		mProjection[3][0], mProjection[3][1], vProjClipPlane.w, mProjection[3][3]);
}

/// Computes the three tangent space vectors of a triangle
void beMath::ComputeTangentSpace(const void *pVertex1, const void *pVertex2, const void *pVertex3, unsigned short nPositionOffset, unsigned short nTexCoordOffset,
								 CVector *pTangent, CVector *pBinormal, CVector *pNormal)
{
	// Get positions and texture coordinates
	const float *pVertex1_Position = reinterpret_cast<const float*>(reinterpret_cast<const char*>(pVertex1) + nPositionOffset);
	const float *pVertex2_Position = reinterpret_cast<const float*>(reinterpret_cast<const char*>(pVertex2) + nPositionOffset);
	const float *pVertex3_Position = reinterpret_cast<const float*>(reinterpret_cast<const char*>(pVertex3) + nPositionOffset);

	const float *pVertex1_TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const char*>(pVertex1) + nTexCoordOffset);
	const float *pVertex2_TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const char*>(pVertex2) + nTexCoordOffset);
	const float *pVertex3_TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const char*>(pVertex3) + nTexCoordOffset);

	// Compute distances
	float fDeltaX1 = pVertex2_Position[0] - pVertex1_Position[0];
	float fDeltaX2 = pVertex3_Position[0] - pVertex1_Position[0];
	float fDeltaY1 = pVertex2_Position[1] - pVertex1_Position[1];
	float fDeltaY2 = pVertex3_Position[1] - pVertex1_Position[1];
	float fDeltaZ1 = pVertex2_Position[2] - pVertex1_Position[2];
	float fDeltaZ2 = pVertex3_Position[2] - pVertex1_Position[2];

	float fDeltaU1 = pVertex2_TexCoord[0] - pVertex1_TexCoord[0];
	float fDeltaU2 = pVertex3_TexCoord[0] - pVertex1_TexCoord[0];
	float fDeltaV1 = pVertex2_TexCoord[1] - pVertex1_TexCoord[1];
	float fDeltaV2 = pVertex3_TexCoord[1] - pVertex1_TexCoord[1];

	// Compute tangent and binormal
	float r = 1.0f / (fDeltaU1 * fDeltaV2 - fDeltaU2 * fDeltaV1);

	CVector vTangent(
		(fDeltaV2 * fDeltaX1 - fDeltaV1 * fDeltaX2) * r,
		(fDeltaV2 * fDeltaY1 - fDeltaV1 * fDeltaY2) * r,
		(fDeltaV2 * fDeltaZ1 - fDeltaV1 * fDeltaZ2) * r);

	CVector vBinormal(
		(fDeltaU1 * fDeltaX2 - fDeltaU2 * fDeltaX1) * r,
		(fDeltaU1 * fDeltaY2 - fDeltaU2 * fDeltaY1) * r,
		(fDeltaU1 * fDeltaZ2 - fDeltaU2 * fDeltaZ1) * r);

	// Assign
	if(pTangent)
		(*pTangent) = Normalize(vTangent);
	if(pBinormal)
		(*pBinormal) = Normalize(vBinormal);
	if(pNormal)
		(*pNormal) = Normalize(Cross(vTangent, vBinormal));
}
*/
} // namespace

#endif