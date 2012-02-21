/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_WRAPPER
#define BE_CORE_WRAPPER

#include "beCore.h"

namespace beCore
{

/// Base class for interface wrappers.
template <class Interface, class Derived>
class Wrapper
{
protected:
	LEAN_INLINE Wrapper() { }
	LEAN_INLINE Wrapper(const Wrapper&) { }
	LEAN_INLINE Wrapper& operator =(const Wrapper&) { return *this; }
#ifndef LEAN_OPTIMIZE_DEFAULT_DESTRUCTOR
	LEAN_INLINE ~Wrapper() throw() { }
#endif
	
public:
	/// Gets the wrapped interface.
	LEAN_INLINE Interface*const& Get() { return static_cast<Derived&>(*this).GetInterface(); }
	
	/// Gets the wrapped interface.
	LEAN_INLINE Interface& operator *() { return *Get(); };
	
	/// Gets the wrapped interface.
	LEAN_INLINE Interface* operator ->() { return Get(); };
	
	/// Gets the wrapped interface.
	LEAN_INLINE operator Interface*() { return Get(); };
};

/// Base class for interface wrappers.
template <class Interface, class Derived>
class TransitiveWrapper
{
protected:
	LEAN_INLINE TransitiveWrapper() { }
	LEAN_INLINE TransitiveWrapper(const TransitiveWrapper&) { }
	LEAN_INLINE TransitiveWrapper& operator =(const TransitiveWrapper&) { return *this; }
#ifndef LEAN_OPTIMIZE_DEFAULT_DESTRUCTOR
	LEAN_INLINE ~TransitiveWrapper() throw() { }
#endif
	
public:
	/// Gets the wrapped interface.
	LEAN_INLINE Interface*const& Get() { return static_cast<Derived&>(*this).GetInterface(); }
	/// Gets the wrapped interface.
	LEAN_INLINE const Interface*const& Get() const { return static_cast<const Derived&>(*this).GetInterface(); }
	
	/// Gets the wrapped interface.
	LEAN_INLINE Interface& operator *() { return *Get(); };
	/// Gets the wrapped interface.
	LEAN_INLINE const Interface& operator *() const { return *Get(); };
	
	/// Gets the wrapped interface.
	LEAN_INLINE Interface* operator ->() { return Get(); };
	/// Gets the wrapped interface.
	LEAN_INLINE const Interface* operator ->() const { return Get(); };

	/// Gets the wrapped interface.
	LEAN_INLINE operator Interface*() { return Get(); };
	/// Gets the wrapped interface.
	LEAN_INLINE operator const Interface*() const { return Get(); };
};

/// Base class for interface wrappers.
template <class Interface, class Derived>
class IntransitiveWrapper
{
protected:
	LEAN_INLINE IntransitiveWrapper() { }
	LEAN_INLINE IntransitiveWrapper(const IntransitiveWrapper&) { }
	LEAN_INLINE IntransitiveWrapper& operator =(const IntransitiveWrapper&) { return *this; }
#ifndef LEAN_OPTIMIZE_DEFAULT_DESTRUCTOR
	LEAN_INLINE ~IntransitiveWrapper() throw() { }
#endif
	
public:
	/// Gets the wrapped interface.
	LEAN_INLINE Interface*const& Get() const { return static_cast<const Derived&>(*this).GetInterface(); }
	
	/// Gets the wrapped interface.
	LEAN_INLINE Interface& operator *() const { return *Get(); };
	
	/// Gets the wrapped interface.
	LEAN_INLINE Interface* operator ->() const { return Get(); };
	
	/// Gets the wrapped interface.
	LEAN_INLINE operator Interface*() const { return Get(); };
};

} // namespace

#endif