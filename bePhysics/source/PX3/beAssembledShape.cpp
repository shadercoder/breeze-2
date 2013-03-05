/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beAssembledShape.h"
#include "bePhysics/PX3/beRigidActors.h"
#include "bePhysics/PX3/beDevice.h"
#include <PxExtensionsAPI.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

// Constructor.
AssembledShape::AssembledShape(physx::PxPhysics &device)
	: m_actorPrototype( CreateRigidDynamic(device) ),
	m_pSerialDataToBeFreed()
{
}

// Constructor.
AssembledShape::AssembledShape(physx::PxRigidDynamic *actor, void *serialDataToBeFreed)
	: m_actorPrototype( LEAN_ASSERT_NOT_NULL(actor) ),
	m_pSerialDataToBeFreed(serialDataToBeFreed)
{
}

// Destructor.
AssembledShape::~AssembledShape()
{
	// ORDER: Actor lies IN the block of memory!
	m_actorPrototype = nullptr;
	PhysXSerializationFree(m_pSerialDataToBeFreed);
}

// Gets the number of shapes.
uint4 AssembledShape::GetShapeCount() const
{
	return m_actorPrototype->getNbShapes();
}

// Gets the name of the n-th shape.
utf8_ntr AssembledShape::GetShapeName(uint4 idx) const
{
	utf8_ntr name;
	physx::PxShape *shape = nullptr;
	m_actorPrototype->getShapes(&shape, 1, idx);
	if (const char *pName = LEAN_ASSERT_NOT_NULL(shape)->getName())
		name = utf8_ntr(pName);
	return name;
}

// Gets the n-th shape.
bePhysics::ShapeHandle AssembledShape::GetShape(uint4 idx) const
{
	physx::PxShape *shape = nullptr;
	m_actorPrototype->getShapes(&shape, 1, idx);
	return ShapeHandle(shape);
}

// Gets the at max given number of shapes.
uint4 AssembledShape::GetShapes(bePhysics::ShapeHandle *shapes, uint4 count, uint4 offset) const
{
	return m_actorPrototype->getShapes(ShapeHandle::Array(shapes), count, offset);
}

} // namespace

// Gets the name of the given shape.
utf8_ntr GetName(ShapeHandle shape)
{
	const char *name = ToImpl(shape)->getName();
	return name ? utf8_ntr(name) : utf8_ntr();
}

} // namespace