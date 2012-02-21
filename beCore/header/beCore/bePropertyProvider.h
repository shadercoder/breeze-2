/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_PROPERTY_PROVIDER
#define BE_CORE_PROPERTY_PROVIDER

#include "beCore.h"
#include "beTypeIndex.h"
#include <lean/type_info.h>

namespace beCore
{

/// Property description.
struct PropertyDesc
{
	const lean::type_info *TypeInfo;	///< Value (component) type.
	size_t Count;						///< Value (component) count.
	uint4 TypeID;						///< Serialization type.

	/// Default Constructor.
	PropertyDesc()
		: TypeInfo(&lean::get_type_info<void>()),
		Count(0),
		TypeID(TypeIndex::InvalidID) { }
	/// Constructor.
	PropertyDesc(const lean::type_info &typeInfo, size_t count, uint4 typeID)
		: TypeInfo(&typeInfo),
		Count(count),
		TypeID(typeID) { }
};

class PropertyVisitor;

/// Generic property provider base class.
class LEAN_INTERFACE PropertyProvider
{
protected:
	PropertyProvider& operator =(const PropertyProvider&) { return *this; }
	~PropertyProvider() throw() { }

public:
	/// Gets the number of properties.
	virtual uint4 GetPropertyCount() const = 0;
	/// Gets the ID of the given property.
	virtual uint4 GetPropertyID(const utf8_ntri &name) const = 0;
	/// Gets the name of the given property.
	virtual utf8_ntr GetPropertyName(uint4 id) const = 0;
	/// Gets the type of the given property.
	virtual PropertyDesc GetPropertyDesc(uint4 id) const = 0;

	/// Sets the given (raw) values.
	virtual bool SetProperty(uint4 id, const std::type_info &type, const void *values, size_t count) = 0;
	/// Gets the given number of (raw) values.
	virtual bool GetProperty(uint4 id, const std::type_info &type, void *values, size_t count) const = 0;

	/// Visits a property for modification.
	virtual bool WriteProperty(uint4 id, PropertyVisitor &visitor, bool bWriteOnly = true) = 0;
	/// Visits a property for reading.
	virtual bool ReadProperty(uint4 id, PropertyVisitor &visitor) const = 0;

	/// Sets the given value.
	template <class Value>
	LEAN_INLINE bool SetProperty(uint4 id, const Value &value) { return SetProperty(id, typeid(Value), lean::addressof(value), 1); }
	/// Sets the given values.
	template <class Value>
	LEAN_INLINE bool SetProperty(uint4 id, const Value *values, size_t count) { return SetProperty(id, typeid(Value), values, count); }
	/// Gets a value.
	template <class Value>
	LEAN_INLINE bool GetProperty(uint4 id, Value &value) const { return GetProperty(id, typeid(Value), lean::addressof(value), 1); }
	/// Gets the given number of values.
	template <class Value>
	LEAN_INLINE bool GetProperty(uint4 id, Value *values, size_t count) const { return GetProperty(id, typeid(Value), values, count); }

	/// Gets the type index.
	virtual const TypeIndex* GetTypeIndex() const = 0;
};

/// Enhanced generic property provider base class.
class EnhancedPropertyProvider : public PropertyProvider
{
protected:
	EnhancedPropertyProvider& operator =(const EnhancedPropertyProvider&) { return *this; }
	~EnhancedPropertyProvider() throw() { }

public:
	/// Resets the given property to its default value.
	virtual bool ResetProperty(size_t id) = 0;

	/// Gets a default value provider.
	virtual const PropertyProvider* GetPropertyDefaults() const = 0;
	/// Gets a range minimum provider.
	virtual const PropertyProvider* GetLowerPropertyLimits() const = 0;
	/// Gets a range maximum provider.
	virtual const PropertyProvider* GetUpperPropertyLimits() const = 0;
};

} // namespace

#endif