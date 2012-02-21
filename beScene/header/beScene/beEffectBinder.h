/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_EFFECT_BINDER
#define BE_SCENE_EFFECT_BINDER

#include "beScene.h"
#include <beCore/beShared.h>
#include <beGraphics/beEffect.h>

namespace beScene
{

/// Effect binder base.
class EffectBinder : public beCore::Shared
{
protected:
	LEAN_INLINE EffectBinder& operator =(const EffectBinder&) { return *this; }
	
public:
	/// Destructor.
	virtual ~EffectBinder() { };

	/// Gets the effect bound.
	virtual const beGraphics::Effect& GetEffect() const = 0;
};

} // namespace

#endif