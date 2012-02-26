/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_SETUP
#define BE_GRAPHICS_SETUP

#include "beGraphics.h"
#include <beCore/beShared.h>
#include <beCore/beReflectedComponent.h>
#include "beTextureProvider.h"
#include "beDeviceContext.h"
#include "beEffect.h"

namespace beGraphics
{

/// Setup interface.
class Setup : public beCore::Resource, public beCore::ReflectedComponent, public TextureProvider, public Implementation
{
protected:
	LEAN_INLINE Setup& operator =(const Setup&) { return *this; }

public:
	virtual ~Setup() throw() { }

	/// Applys the setup.
	virtual void Apply(const DeviceContext &context) const = 0;

	/// Gets the effect.
	virtual const Effect* GetEffect() const = 0;
};

} // namespace

#endif