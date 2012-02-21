/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERJOB
#define BE_SCENE_RENDERJOB

#include "beScene.h"
#include <lean/tags/transitive_ptr.h>
#include <beMath/beSphereDef.h>

namespace beScene
{

// Prototypes.
class Perspective;
struct LightJob;
struct RenderJob;
class RenderContext;

/// Render function type.
typedef void (RenderFunction)(const RenderJob &job, const Perspective &perspective,
		const LightJob *lights, const LightJob *lightsEnd, const RenderContext &context);

/// Renderable pass structure.
struct RenderablePass
{
	const void *PassData;	///< Pass-specific data.
	uint4 SortIndex;		///< Sort index.
	uint2 StageID;			///< Pipeline stage index.
	uint2 QueueID;			///< Render queue index.
	uint4 Flags;			///< Flags.

	/// Default constructor.
	LEAN_INLINE RenderablePass()
		: PassData(nullptr),
		SortIndex(0),
		StageID( static_cast<uint2>(-1) ),
		QueueID( static_cast<uint2>(-1) ),
		Flags(0) { }
	/// Constructor.
	LEAN_INLINE RenderablePass(const void *pPassData,
		uint4 sortIndex,
		uint2 stageID, uint2 queueID,
		uint4 flags)
			: PassData(pPassData),
			SortIndex(sortIndex),
			StageID(stageID),
			QueueID(queueID),
			Flags(flags) { }
};

/// Renderable data structure.
struct RenderableData
{
	beMath::fsphere3 Bounds;						///< Bounds.
	RenderFunction *Render;							///< Render function.
	const void *SharedData;							///< Shared renderable data.
	lean::transitive_ptr<RenderablePass> Passes;	///< Passes.
	uint4 PassCount;								///< Number of passes.

	/// Default constructor.
	LEAN_INLINE RenderableData()
		: Render(nullptr),
		SharedData(nullptr),
		Passes(nullptr),
		PassCount(0) { }
	/// Constructor.
	LEAN_INLINE RenderableData(RenderFunction *pRender,
		const void *pSharedData,
		RenderablePass *pPasses,
		uint4 passCount)
			: Render(pRender),
			SharedData(pSharedData),
			Passes(pPasses),
			PassCount(passCount) { }
};

/// Render job structure.
struct RenderJob
{
	RenderFunction *Render;			///< Render function.
	const void *SharedData;			///< Shared renderable data.
	const void *PassData;			///< Pass-specific data.
	void *PerspectiveData;			///< Perspective-specific data.
	
	/// NON-INITIALIZING constructor.
	LEAN_INLINE RenderJob() { }
	/// Constructor.
	LEAN_INLINE RenderJob(RenderFunction *pRender,
		const void *pSharedData,
		const void *pPassData,
		void *pPerspectiveData)
			: Render(pRender),
			SharedData(pSharedData),
			PassData(pPassData),
			PerspectiveData(pPerspectiveData) { }
	/// Constructor.
	LEAN_INLINE RenderJob(const RenderableData &data, const RenderablePass &pass, void *pPerspectiveData)
		: Render(data.Render),
		SharedData(data.SharedData),
		PassData(pass.PassData),
		PerspectiveData(pPerspectiveData) { }
};

} // namespace

#endif