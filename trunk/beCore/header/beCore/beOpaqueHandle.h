/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_OPAQUE_HANDLE
#define BE_CORE_OPAQUE_HANDLE

#include "beCore.h"
#include "beWrapper.h"

namespace beCore
{

/// Opaque handle interface.
template <class Tag>
class OpaqueHandle
{
protected:
	void *m_handle;

	/// Constructor.
	LEAN_INLINE OpaqueHandle(void *handle)
		: m_handle(handle) { }

	/// Converts the handle into a pointer of the given type.
	template <class Type>
	LEAN_INLINE Type*const& To() const { return reinterpret_cast<Type*const&>(m_handle); }

public:
	/// Copy constructor.
	LEAN_INLINE OpaqueHandle(const OpaqueHandle &right)
		: m_handle(right.m_handle) { }
	/// Copy assignment operator.
	LEAN_INLINE OpaqueHandle& operator =(const OpaqueHandle &right)
	{
		m_handle = right.m_handle;
		return *this;
	}
};

/// Qualified handle declaration.
template <class Tag>
class QualifiedHandle;

/// Default qualified handle implementation.
#define BE_CORE_DEFINE_QUALIFIED_HANDLE(Tag, Interface, Wrapper)						\
	template <>																			\
	class beCore::QualifiedHandle<Tag>													\
		: public Wrapper< Interface, beCore::QualifiedHandle<Tag> >,					\
		public beCore::OpaqueHandle<Tag>												\
	{																					\
	private:																			\
		friend class Wrapper< Interface, beCore::QualifiedHandle<Tag> >;				\
																						\
		LEAN_INLINE Interface*const& GetInterface() const { return To<Interface>(); }	\
																						\
	public:																				\
		QualifiedHandle(Interface *pInterface = nullptr)								\
			: OpaqueHandle<Tag>(pInterface) { }											\
	};

/// Casts the given opaque handle to its qualified equivalent.
template <class Tag>
LEAN_INLINE QualifiedHandle<Tag>* ToImpl(OpaqueHandle<Tag> *handle)
{
	return static_cast< QualifiedHandle<Tag>* >(handle);
}
/// Casts the given opaque handle to its qualified equivalent.
template <class Tag>
LEAN_INLINE const QualifiedHandle<Tag>* ToImpl(const OpaqueHandle<Tag> *handle)
{
	return static_cast< const QualifiedHandle<Tag>* >(handle);
}
/// Casts the given opaque handle to its qualified equivalent.
template <class Tag>
LEAN_INLINE QualifiedHandle<Tag>& ToImpl(OpaqueHandle<Tag> &handle)
{
	return *ToImpl(&handle);
}
/// Casts the given opaque handle to its qualified equivalent.
template <class Tag>
LEAN_INLINE const QualifiedHandle<Tag>& ToImpl(const OpaqueHandle<Tag> &handle)
{
	return *ToImpl(&handle);
}

} // namespace

#endif