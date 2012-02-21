/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_INLINE_MATERIAL_SERIALIZATION
#define BE_SCENE_INLINE_MATERIAL_SERIALIZATION

#include "beScene.h"
#include "beMaterial.h"
#include <beCore/beParameterSet.h>
#include <beEntitySystem/beSerializationJob.h>

namespace beScene
{

/// Schedules the given material for inline serialization.
BE_SCENE_API void SaveMaterial(const Material *pMaterial,
	beCore::ParameterSet &parameters, beEntitySystem::SerializationQueue<beEntitySystem::SaveJob> &queue);

} // namespace

#endif