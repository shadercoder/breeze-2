/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_EFFECT
#define BE_GRAPHICS_EFFECT

#include "beGraphics.h"
#include <beCore/beShared.h>

namespace beGraphics
{

class EffectCache;

/// Effect interface.
class Effect : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE Effect& operator =(const Effect&) { return *this; }

public:
	virtual ~Effect() throw() { };

	/// Gets the effect cache.
	virtual EffectCache* GetCache() const = 0;
};

/// Technique interface.
class Technique : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE Technique& operator =(const Technique&) { return *this; }

public:
	virtual ~Technique() throw() { };

	/// Gets the effect.
	virtual const Effect* GetEffect() const = 0;
};

} // namespace

#endif