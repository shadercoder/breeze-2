/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_PROPERTY_LISTENER
#define BE_CORE_PROPERTY_LISTENER

#include "beCore.h"
#include "bePropertyProvider.h"

namespace beCore
{

/// Property listener.
class LEAN_INTERFACE PropertyListener
{
protected:
	~PropertyListener() { }

public:
	/// Called when the given propery might have changed.
	virtual void PropertyChanged(const PropertyProvider &provider) = 0;
};

} // namespace

#endif