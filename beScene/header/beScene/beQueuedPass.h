/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_QUEUED_PASS
#define BE_SCENE_QUEUED_PASS

#include "beScene.h"

namespace beScene
{

/// Queued effect binder pass.
class QueuedPass
{
protected:
	LEAN_INLINE QueuedPass& operator =(const QueuedPass&) { return *this; }
	LEAN_INLINE ~QueuedPass() throw() { }

public:
	/// Gets the stage ID.
	virtual uint4 GetStageID() const = 0;
	/// Gets the queue ID.
	virtual uint4 GetQueueID() const = 0;

	/// Gets the input signature of this pass.
	virtual const char* GetInputSignature(uint4 &size) const = 0;
};

} // namespace

#endif