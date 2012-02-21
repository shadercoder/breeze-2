/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_MEMORY
#define BE_GRAPHICS_MEMORY

#include "beGraphics.h"
#include <beCore/beShared.h>

namespace beGraphics
{

/// Memory interface.
template <class Element>
class Memory : public beCore::COMResource
{
protected:
	LEAN_INLINE Memory& operator =(const Memory&) { return *this; }
	LEAN_INLINE ~Memory() throw() { }

public:
	/// Gets a pointer to the data buffer.
	virtual Element* GetData() = 0;
	/// Gets a pointer to the data buffer.
	virtual const Element* GetData() const = 0;
	/// Gets the data buffer size (in elements).
	virtual size_t GetSize() const = 0;
};

}

#endif