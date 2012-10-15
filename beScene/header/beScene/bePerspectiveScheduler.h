/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PERSPECTIVE_SCHEDULER
#define BE_SCENE_PERSPECTIVE_SCHEDULER

#include "beScene.h"
#include "beRenderingLimits.h"

namespace beScene
{
	
// Prototypes
class Perspective;
struct PerspectiveDesc;
class Pipe;
class PipelineProcessor;

/// Perspective scheduler interface.
class PerspectiveScheduler
{
protected:
	LEAN_INLINE PerspectiveScheduler& operator=(const PerspectiveScheduler&) { return *this; }
	LEAN_INLINE ~PerspectiveScheduler() throw() { }

public:
	/// Adds a perspective.
	virtual Perspective* AddPerspective(const PerspectiveDesc &desc, Pipe *pPipe,
		PipelineProcessor *pProcessor = nullptr, PipelineStageMask stageMask = 0, bool bNormalOnly = false) = 0;
};

} // nmaespace

#endif