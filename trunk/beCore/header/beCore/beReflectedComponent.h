/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_REFLECTED_COMPONENT
#define BE_CORE_REFLECTED_COMPONENT

#include "beCore.h"
#include "bePropertyProvider.h"

#include <lean/containers/any.h>
#include <lean/smart/cloneable_obj.h>

namespace beCore
{

/// Reflected component base class.
class LEAN_INTERFACE ReflectedComponent : public PropertyProvider
{
protected:
	ReflectedComponent& operator =(const ReflectedComponent&) { return *this; }
	~ReflectedComponent() throw() { }

public:
	/// Gets the number of child components.
	virtual uint4 GetComponentCount() const = 0;
	/// Gets the name of the n-th child component.
	virtual Exchange::utf8_string GetComponentName(uint4 idx) const { return GetComponentType(idx); }
	/// Gets the n-th child property provider, nullptr if not a property provider.
	virtual PropertyProvider* GetPropertyProvider(uint4 idx)
	{
		return const_cast<PropertyProvider*>( const_cast<const ReflectedComponent*>(this)->GetPropertyProvider(idx) );
	}
	/// Gets the n-th child property provider, nullptr if not a property provider.
	virtual const PropertyProvider* GetPropertyProvider(uint4 idx) const { return GetReflectedComponent(idx); }
	/// Gets the n-th reflected child component, nullptr if not reflected.
	virtual ReflectedComponent* GetReflectedComponent(uint4 idx)
	{
		return const_cast<ReflectedComponent*>( const_cast<const ReflectedComponent*>(this)->GetReflectedComponent(idx) );
	}
	/// Gets the n-th reflected child component, nullptr if not reflected.
	virtual const ReflectedComponent* GetReflectedComponent(uint4 idx) const = 0;

	/// Gets the type of the n-th child component.
	virtual Exchange::utf8_string GetComponentType(uint4 idx) const = 0;
	/// Gets the n-th component.
	virtual lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const = 0;

	/// Returns true, if the n-th component can be replaced.
	virtual bool IsComponentReplaceable(uint4 idx) const = 0;
	/// Sets the n-th component.
	virtual void SetComponent(uint4 idx, const lean::any &pComponent) = 0;
};

/// Reflected component base class.
template <class Interface = ReflectedComponent>
class LEAN_INTERFACE RigidReflectedComponent : public Interface
{
protected:
	RigidReflectedComponent& operator =(const RigidReflectedComponent&) { return *this; }
	~RigidReflectedComponent() throw() { }

public:
	/// Gets the name of the n-th child component.
	virtual Exchange::utf8_string GetComponentName(uint4 idx) const = 0;

	/// Gets the type of the n-th child component.
	virtual Exchange::utf8_string GetComponentType(uint4 idx) const { return Exchange::utf8_string(); }
	/// Gets the n-th component.
	virtual lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const { return nullptr; }
	
	/// Returns true, if the n-th component can be replaced.
	virtual bool IsComponentReplaceable(uint4 idx) const { return false; }
	/// Sets the n-th component.
	virtual void SetComponent(uint4 idx, const lean::any &pComponent) { }
};

/// Reflected component base class.
template <class Interface = ReflectedComponent>
class LEAN_INTERFACE ReflectedPropertyProvider : public RigidReflectedComponent<Interface>
{
protected:
	ReflectedPropertyProvider& operator =(const ReflectedPropertyProvider&) { return *this; }
	~ReflectedPropertyProvider() throw() { }

public:
	/// Gets the number of child components.
	virtual uint4 GetComponentCount() const { return 0; }
	/// Gets the name of the n-th child component.
	virtual Exchange::utf8_string GetComponentName(uint4 idx) const { return GetComponentType(idx); }
	/// Gets the n-th reflected child component, nullptr if not reflected.
	virtual const ReflectedComponent* GetReflectedComponent(uint4 idx) const { return nullptr; }
};

} // namespace

#endif