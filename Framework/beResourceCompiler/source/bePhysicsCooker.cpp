/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/bePhysicsCookerImpl.h"
#include <bePhysics/PX/beDevice.h>
#include <lean/logging/errors.h>

namespace beResourceCompiler
{

namespace
{

PhysicsCooker::Data* CreateCookerData()
{
	bePhysics::scoped_pxptr_t<physx::PxFoundation>::t pFoundation( bePhysics::CreateFoundation() );
	bePhysics::scoped_pxptr_t<physx::PxPhysics>::t pPhysics( bePhysics::CreatePhysics(*pFoundation) );

	bePhysics::scoped_pxptr_t<physx::PxCooking>::t pCooker(
			PxCreateCooking(PX_PHYSICS_VERSION, pPhysics->getFoundation(), physx::PxCookingParams())
		);

	if (!pCooker)
		LEAN_THROW_ERROR_MSG("PxCreateCooking()");

	return new PhysicsCooker::Data(pPhysics.detach(), pCooker.detach());
}

} // namespace

// Constructor.
PhysicsCooker::PhysicsCooker()
	: m_data( CreateCookerData() )
{
}

// Destructor.
PhysicsCooker::~PhysicsCooker()
{
}

} // namespace
