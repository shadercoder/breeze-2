/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/beReflectionProperties.h"
#include "beCore/beReflectionPropertyProvider.h"

#include "beCore/bePropertyVisitor.h"

namespace beCore
{

// Gets the number of properties.
uint4 ReflectionPropertyProvider::GetPropertyCount() const
{
	return static_cast<uint4>( GetReflectionProperties().size() );
}

// Gets the ID of the given property.
uint4 ReflectionPropertyProvider::GetPropertyID(const utf8_ntri &name) const
{
	return static_cast<uint4>( lean::find_property(GetReflectionProperties(), name, InvalidPropertyID) );
}

// Gets the name of the given property.
utf8_ntr ReflectionPropertyProvider::GetPropertyName(uint4 id) const
{
	Properties properties = GetReflectionProperties();
	return (id < properties.size()) ? utf8_ntr(properties[id].name) : utf8_ntr("");
}

// Gets the type of the given property.
PropertyDesc ReflectionPropertyProvider::GetPropertyDesc(uint4 id) const
{
	PropertyDesc desc;

	Properties properties = GetReflectionProperties();

	if (id < properties.size())
	{
		const ReflectionProperty &property = properties[id];
		desc = PropertyDesc(*property.type_info, property.count, property.typeID, property.widget);
	}

	return desc;
}

/// Sets the given (raw) values.
bool ReflectionPropertyProvider::SetProperty(uint4 id, const std::type_info &type, const void *values, size_t count)
{
	Properties properties = GetReflectionProperties();

	if (id < properties.size())
	{
		const ReflectionProperty &desc = properties[id];

		if (desc.setter.valid())
			if ((*desc.setter)(*this, type, values, desc.count))
			{
				EmitPropertyChanged();
				return true;
			}
	}

	return false;
}
/// Gets the given number of (raw) values.
bool ReflectionPropertyProvider::GetProperty(uint4 id, const std::type_info &type, void *values, size_t count) const
{
	Properties properties = GetReflectionProperties();

	if (id < properties.size())
	{
		const ReflectionProperty &desc = properties[id];

		if (desc.getter.valid())
			return (*desc.getter)(*this, type, values, desc.count);
	}

	return false;
}

/// Visits a property for modification.
bool ReflectionPropertyProvider::WriteProperty(uint4 id, PropertyVisitor &visitor, bool bWriteOnly)
{
	Properties properties = GetReflectionProperties();

	if (id < properties.size())
	{
		const ReflectionProperty &desc = properties[id];

		if (desc.setter.valid())
		{
			const lean::property_type &propertyType = *desc.type_info->property_type;
			size_t size = propertyType.size(desc.count);

			static const size_t StackBufferSize = 16 * 8;
			char stackBuffer[StackBufferSize];

			lean::scoped_property_data<lean::deallocate_property_data_policy> bufferGuard(
				propertyType, (size <= StackBufferSize) ? nullptr : propertyType.allocate(desc.count), desc.count );
			void *values = (bufferGuard.data()) ? bufferGuard.data() : stackBuffer;

			propertyType.construct(values, desc.count);
			lean::scoped_property_data<lean::destruct_property_data_policy> valueGuard(propertyType, values, desc.count);

			if (bWriteOnly || (*desc.getter)(*this, desc.type_info->type, values, desc.count))
			{
				bool bModified = visitor.Visit(*this, id, PropertyDesc(*desc.type_info, desc.count, desc.typeID, desc.widget), values);

				if (bModified)
				{
					if ((*desc.setter)(*this, desc.type_info->type, values, desc.count))
					{
						EmitPropertyChanged();
						return true;
					}
				}
				
				return !bModified;
			}
		}
	}

	return false;
}
/// Visits a property for reading.
bool ReflectionPropertyProvider::ReadProperty(uint4 id, PropertyVisitor &visitor, bool bPersistentOnly) const
{
	Properties properties = GetReflectionProperties();

	if (id < properties.size())
	{
		const ReflectionProperty &desc = properties[id];

		if (desc.getter.valid() && (!bPersistentOnly || desc.persistent))
		{
			const lean::property_type &propertyType = *desc.type_info->property_type;
			size_t size = propertyType.size(desc.count);

			static const size_t StackBufferSize = 16 * 8;
			char stackBuffer[StackBufferSize];

			lean::scoped_property_data<lean::deallocate_property_data_policy> bufferGuard(
				propertyType, (size <= StackBufferSize) ? nullptr : propertyType.allocate(desc.count), desc.count );
			void *values = (bufferGuard.data()) ? bufferGuard.data() : stackBuffer;

			propertyType.construct(values, desc.count);
			lean::scoped_property_data<lean::destruct_property_data_policy> valueGuard(propertyType, values, desc.count);

			if ((*desc.getter)(*this, desc.type_info->type, values,  desc.count))
			{
				// WARNING: Call read-only overload!
				visitor.Visit(*this, id, PropertyDesc(*desc.type_info, desc.count, desc.typeID, desc.widget), const_cast<const void*>(values));
				return true;
			}
		}
	}

	return false;
}

/// Gets the type index.
const TypeIndex* ReflectionPropertyProvider::GetPropertyTypeIndex() const
{
	return &beCore::GetTypeIndex();
}

} // namespace
