/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beScene.h"
#include "bePhysics/PX/beDevice.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

physx::PxFilterFlags DefaultSimulationFilterShader(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0, 
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	physx::PxU32 constantBlockSize)
{
	bool bIsTrigger = physx::PxFilterObjectIsTrigger(attributes0) | physx::PxFilterObjectIsTrigger(attributes1);

	if (bIsTrigger)
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	else
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	return bIsTrigger | (filterData0.word3 == 0) | (filterData1.word3 == 0) | ((filterData0.word3 & filterData1.word3) != 0)
		? physx::PxFilterFlag::eDEFAULT
		: physx::PxFilterFlag::eSUPPRESS;
}

// Creates a physics scene.
physx::PxScene* CreateScene(physx::PxPhysics &physics, const physx::PxSceneDesc &desc)
{
	physx::PxSceneDesc enhancedDesc(desc);

	if(!enhancedDesc.filterShader)
		enhancedDesc.filterShader = &DefaultSimulationFilterShader;

	physx::PxScene *pScene = physics.createScene(enhancedDesc);

	if (!pScene)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createScene()");

	return pScene;
}

// Constructor.
ScenePX::ScenePX(Device &device, const SceneDesc &desc)
	: m_pScene(
		CreateScene(
			*ToImpl(device),
			ToImpl(
				desc,
				ToImpl(device)->getTolerancesScale(),
				ToImpl(device).GetCPUDispatcher(),
				ToImpl(device).GetGPUDispatcher() )
		)
	)
{
}

// Constructor.
ScenePX::ScenePX(physx::PxScene *pScene)
	: m_pScene( LEAN_ASSERT_NOT_NULL(pScene) )
{
}

// Destructor.
ScenePX::~ScenePX()
{
}

} // namespace