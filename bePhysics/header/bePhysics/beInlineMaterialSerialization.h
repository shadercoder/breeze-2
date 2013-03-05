/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS_INLINE_MATERIAL_SERIALIZATION
#define BE_PHYSICS_INLINE_MATERIAL_SERIALIZATION

#include "bePhysics.h"
#include "beMaterial.h"
#include <beCore/beParameterSet.h>
#include <beCore/beSerializationJobs.h>

namespace bePhysics
{

/// Schedules the given material for inline serialization.
BE_PHYSICS_API void SaveMaterial(const Material *pMaterial,
	beCore::ParameterSet &parameters, beCore::SerializationQueue<beCore::SaveJob> &queue);

} // namespace

#endif