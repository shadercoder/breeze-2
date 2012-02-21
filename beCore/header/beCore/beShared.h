/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_SHARED
#define BE_CORE_SHARED

#include "beCore.h"
#include <lean/memory/win_heap.h>
#include <lean/memory/heap_bound.h>
#include <lean/memory/heap_allocator.h>
#include <lean/smart/resource.h>

namespace beCore
{

/// Provides complex types that may be shared across module boundaries.
namespace Exchange
{
	/// Exchange heap.
	typedef lean::win_heap exchange_heap;
	
	/// Defines an allocator type that may be used in STL-conformant containers intended for cross-module data exchange.
	template <class Type, size_t Alignment = alignof(Type)>
	struct exchange_allocator_t
	{
		/// Exchange allocator type.
		typedef lean::heap_allocator<Type, exchange_heap, Alignment> t;
	};

} // namespace

using Exchange::exchange_heap;
using Exchange::exchange_allocator_t;

/// Shared object base class.
typedef lean::heap_bound<exchange_heap> Shared;

/// Shared resource base class.
class Resource : public Shared,
	public lean::resource<long, exchange_allocator_t<long>::t>
{
protected:
	LEAN_INLINE Resource() { }
	LEAN_INLINE Resource(const Resource&) { }
	LEAN_INLINE Resource& operator =(const Resource&) { return *this; }
	LEAN_INLINE ~Resource() throw() { }
};

/// Shared optional resource base class.
class OptionalResource : public Shared,
	public lean::resource<long, exchange_allocator_t<long>::t, true>
{
protected:
	LEAN_INLINE OptionalResource() { }
	LEAN_INLINE OptionalResource(const OptionalResource&) { }
	LEAN_INLINE OptionalResource& operator =(const OptionalResource&) { return *this; }
	LEAN_INLINE ~OptionalResource() throw() { }
};

/// Shared resource interface.
class ResourceInterface : public Shared,
	public lean::resource_interface<long, exchange_allocator_t<long>::t>
{
protected:
	LEAN_INLINE ResourceInterface& operator =(const ResourceInterface&) { return *this; }
	LEAN_INLINE ~ResourceInterface() throw() { }
};

/// 'COM' resource interface.
class COMResource : public Shared
{
protected:
	LEAN_INLINE ~COMResource() throw() { }

public:
	/// Increses the reference count of this resource.
	virtual void AddRef() const = 0;
	/// Decreases the reference count of this resource, destroying the resource when the count reaches zero.
	virtual void Release() const = 0;
};

} // namespace

#endif