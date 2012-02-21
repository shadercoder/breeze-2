/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE
#define BE_SCENE_RENDERABLE

#include "beScene.h"
#include "beRenderJob.h"

namespace beScene
{

// Prototypes.
class Perspective;
class PerspectiveScheduler;
class DynamicScenery;
class RenderableDataAllocator;

/// Renderable flags enumeration.
namespace RenderableFlags
{
	/// Enumeration
	enum T
	{
		PerspectivePrepare = 0x1,	///< Indicates @code Prepare(Perspective*)@endcode should be called.
		PerspectiveFinalize = 0x2,	///< Indicates @code Finalize(Perspective*)@endcode should be called.
		
		Lit = 0x4					///< Indicates affecting lights should be queried and passed.
	};
}

/// Renderable base.
class LEAN_INTERFACE Renderable
{
protected:
	Renderable& operator =(const Renderable&) { return *this; }
	~Renderable() { }

public:
	/// Called when a renderable is attached to a scenery.
	virtual void Attached(DynamicScenery *pScenery, RenderableData &data, RenderableDataAllocator &allocator) = 0;
	/// Called for each pass when a renderable is attached to a scenery.
	virtual void Attached(DynamicScenery *pScenery, const RenderableData &data, RenderablePass &pass, uint4 passIdx, RenderableDataAllocator &allocator) = 0;
	/// Called when a renderable is detached from a scenery.
	virtual void Detached(DynamicScenery *pScenery) { }

	/// Gets the sort index.
	virtual uint4 GetSortIndex() const = 0;
	/// Gets the number of passes.
	virtual uint4 GetPassCount() const = 0;

	/// Prepares this renderable object.
	virtual void* Prepare(Perspective &perspective, PerspectiveScheduler &perspectiveScheduler) const { return nullptr; }
	/// Prepares this renderable object.
	virtual void Finalize(Perspective &perspective, void *pData) const { }
};

} // namespace

#endif