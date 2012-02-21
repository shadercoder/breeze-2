/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE_EFFECT_DATA
#define BE_SCENE_RENDERABLE_EFFECT_DATA

#include "beScene.h"
#include <beMath/beMatrixDef.h>

namespace beScene
{

/// Typical renderable data structure.
struct RenderableEffectData
{
	beMath::fmat4 Transform;	///< Transformation matrix.
	beMath::fmat4 TransformInv;	///< Inverse transformation matrix.
	uint4 ID;					///< Renderable ID.

	/// Default constructor.
	RenderableEffectData()
		: ID( static_cast<uint4>(-1) ) { }
};

} // namespace

#endif