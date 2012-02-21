/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_EFFECT_DRIVER
#define BE_SCENE_EFFECT_DRIVER

#include "beScene.h"
#include "beEffectBinder.h"
#include <beCore/beShared.h>

namespace beScene
{

/// Effect binder base.
class EffectDriver : public beCore::Resource, public EffectBinder
{
protected:
	LEAN_INLINE EffectBinder& operator =(const EffectBinder&) { return *this; }
};

} // namespace

#endif