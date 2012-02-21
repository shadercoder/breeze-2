/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#ifndef BE_RESOURCECOMPILER_PHYSICSCOOKER_PX
#define BE_RESOURCECOMPILER_PHYSICSCOOKER_PX

#include "beResourceCompiler.h"
#include "bePhysicsCooker.h"
#include <bePhysics/PX/beAPI.h>

namespace beResourceCompiler
{

/// Physics cooker data.
struct PhysicsCooker::Data
{
	bePhysics::scoped_pxptr_t<physx::PxPhysics>::t Physics;
	bePhysics::scoped_pxptr_t<physx::PxCooking>::t Cooking;

	/// Constructor.
	Data(physx::PxPhysics *pPhysics,
			physx::PxCooking *pCooking)
		: Physics(pPhysics),
		Cooking(pCooking) { }
};

} // namespace

#endif