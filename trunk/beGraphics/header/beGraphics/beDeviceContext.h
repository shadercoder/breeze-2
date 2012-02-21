/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_DEVICE_CONTEXT
#define BE_GRAPHICS_DEVICE_CONTEXT

#include "beGraphics.h"
#include <beCore/beShared.h>

namespace beGraphics
{

/// Device context interface.
class DeviceContext : public beCore::Shared, public Implementation
{
protected:
	LEAN_INLINE DeviceContext& operator =(const DeviceContext&) { return *this; }

public:
	virtual ~DeviceContext() throw() { }
};

} // namespace

#endif