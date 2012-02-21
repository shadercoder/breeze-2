/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_EFFECTBINDERCACHE
#define BE_SCENE_EFFECTBINDERCACHE

#include "beScene.h"
#include <beCore/beShared.h>
#include <beGraphics/beEffect.h>

namespace beScene
{

/// Effect binder cache interface.
template <class EffectBinder>
class EffectBinderCache : public lean::noncopyable, public beCore::Resource
{
public:
	virtual ~EffectBinderCache() throw() { }

	/// Cached effect binder type.
	typedef EffectBinder EffectBinder;

	/// Gets an effect binder from the given effect.
	virtual EffectBinder* GetEffectBinder(const beGraphics::Technique &technique, uint4 flags = 0) = 0;
};

} // namespace

#endif