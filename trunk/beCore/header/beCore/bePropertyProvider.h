/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_PROPERTY_PROVIDER
#define BE_CORE_PROPERTY_PROVIDER

#include "beCore.h"
#include "beTypeIndex.h"
#include "beExchangeContainers.h"
#include <lean/properties/property_type.h>

#include <forward_list>

namespace beCore
{

/// Widget enumeration.
namespace Widget
{
	/// Enumeration.
	enum T
	{
		None,			///< No widget.
		Raw,			///< Raw value widget.
		Slider,			///< Slider widget.
		Color,			///< Color widget.
		Angle,			///< Angle widget.
		Orientation,	///< Orientation widget.

		End
	};
}

/// Property description.
struct PropertyDesc
{
	const lean::property_type_info *TypeInfo;	///< Value (component) type.
	uint4 Count;								///< Value (component) count.
	uint2 TypeID;								///< Serialization type.
	int2 Widget;								///< UI widget.

	/// Default Constructor.
	PropertyDesc()
		: TypeInfo(nullptr),
		Count(0),
		TypeID(TypeIndex::InvalidShortID),
		Widget(Widget::None) { }
	/// Constructor. Truncates type ID to 2 bytes.
	PropertyDesc(const lean::property_type_info &typeInfo, uint4 count, uint4 typeID, int2 widget)
		: TypeInfo(&typeInfo),
		Count(count),
		// MONITOR: Type ID truncated to two bytes.
		TypeID(static_cast<uint2>(typeID)),
		Widget(widget) { }
};

class PropertyVisitor;
class PropertyListener;

/// Generic property provider base class.
class LEAN_INTERFACE PropertyProvider
{
protected:
	PropertyProvider& operator =(const PropertyProvider&) { return *this; }
	~PropertyProvider() throw() { }

public:
	/// Invalid property ID.
	static const uint4 InvalidID = static_cast<uint4>(-1);

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
	virtual bool ReadProperty(uint4 id, PropertyVisitor &visitor, bool bPersistentOnly = false) const = 0;

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

	/// Adds a property listener.
	virtual void AddPropertyListener(PropertyListener *listener) = 0;
	/// Removes a property listener.
	virtual void RemovePropertyListener(PropertyListener *pListener) = 0;

	/// Gets the type index.
	virtual const TypeIndex* GetPropertyTypeIndex() const = 0;
};

/// Simple property listener callback implementation.
class PropertyListenerCollection
{
private:
	typedef std::forward_list<PropertyListener*> listener_list;
	listener_list m_listeners;

public:
	/// Constructor.
	BE_CORE_API PropertyListenerCollection();
	/// Does nothing.
	LEAN_INLINE PropertyListenerCollection(const PropertyListenerCollection&) { }
	/// Destructor.
	BE_CORE_API ~PropertyListenerCollection();

	/// Does nothing.
	LEAN_INLINE PropertyListenerCollection& operator =(const PropertyListenerCollection&) { return *this; }

	/// Adds a property listener.
	BE_CORE_API void AddPropertyListener(PropertyListener *listener);
	/// Removes a property listener.
	BE_CORE_API void RemovePropertyListener(PropertyListener *pListener);
	/// Calls all property listeners.
	BE_CORE_API void EmitPropertyChanged(const PropertyProvider &provider) const;

	/// Checks, if any property listeners have been registered, before making the call.
	LEAN_INLINE void RarelyEmitPropertyChanged(const PropertyProvider &provider) const
	{
		if (!m_listeners.empty())
			EmitPropertyChanged(provider);
	}
};

/// Simple property listener callback implementation.
template <class Interface = PropertyProvider>
class LEAN_INTERFACE PropertyFeedbackProvider : public Interface
{
private:
	PropertyListenerCollection m_listeners;

protected:
	PropertyFeedbackProvider& operator =(const PropertyFeedbackProvider&) { return *this; }
	~PropertyFeedbackProvider() throw() { }

public:
	/// Adds a property listener.
	void AddPropertyListener(PropertyListener *listener) { m_listeners.AddPropertyListener(listener); }
	/// Removes a property listener.
	void RemovePropertyListener(PropertyListener *pListener) { m_listeners.RemovePropertyListener(pListener); }
	
	/// Checks, if any property listeners have been registered, before making the call.
	LEAN_INLINE void EmitPropertyChanged() const { m_listeners.RarelyEmitPropertyChanged(*this); }
};

/// Generic property provider base class.
template <class Interface = PropertyProvider>
class LEAN_INTERFACE OptionalPropertyProvider : public PropertyFeedbackProvider<Interface>
{
protected:
	OptionalPropertyProvider& operator =(const OptionalPropertyProvider&) { return *this; }
	~OptionalPropertyProvider() throw() { }

public:
	/// Gets the number of properties.
	virtual uint4 GetPropertyCount() const { return 0; }
	/// Gets the ID of the given property.
	virtual uint4 GetPropertyID(const utf8_ntri &name) const { return InvalidID; }
	/// Gets the name of the given property.
	virtual utf8_ntr GetPropertyName(uint4 id) const { return utf8_ntr(""); }
	/// Gets the type of the given property.
	virtual PropertyDesc GetPropertyDesc(uint4 id) const { return PropertyDesc(); }

	/// Sets the given (raw) values.
	virtual bool SetProperty(uint4 id, const std::type_info &type, const void *values, size_t count) { return false; }
	/// Gets the given number of (raw) values.
	virtual bool GetProperty(uint4 id, const std::type_info &type, void *values, size_t count) const { return false; }

	/// Visits a property for modification.
	virtual bool WriteProperty(uint4 id, PropertyVisitor &visitor, bool bWriteOnly = true) { return false; }
	/// Visits a property for reading.
	virtual bool ReadProperty(uint4 id, PropertyVisitor &visitor, bool bPersistentOnly = false) const { return false; }

	/// Gets the type index.
	virtual const TypeIndex* GetPropertyTypeIndex() const { return nullptr; }
};

/// Enhanced generic property provider base class.
class LEAN_INTERFACE EnhancedPropertyProvider : public PropertyProvider
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

/// Transfers all from the given source property provider to the given destination property provider.
BE_CORE_API void TransferProperties(PropertyProvider &dest, const PropertyProvider &source);

} // namespace

#endif