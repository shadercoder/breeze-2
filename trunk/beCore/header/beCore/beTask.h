/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_TASK
#define BE_CORE_TASK

#include "beCore.h"

namespace beCore
{

/// Task interface.
class Task
{
protected:
#ifndef LEAN_OPTIMIZE_DEFAULT_DESTRUCTOR
	LEAN_INLINE ~Task() { }
#endif

public:
	/// Runs the task.
	BE_CORE_API virtual void Run() = 0;
	/// Calls Run() to run the task.
	LEAN_INLINE void operator ()() { Run(); }
};

} // namespace

#endif