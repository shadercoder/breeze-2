/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/bePropertyProvider.h"

#include "beCore/bePropertyListener.h"

#include "beCore/bePropertyVisitor.h"
#include <lean/functional/predicates.h>

namespace beCore
{

// Constructor.
PropertyListenerCollection::PropertyListenerCollection()
{
}

// Destructor.
PropertyListenerCollection::~PropertyListenerCollection()
{
}

// Adds a property listener.
void PropertyListenerCollection::AddPropertyListener(PropertyListener *listener)
{
	m_listeners.push_front( LEAN_ASSERT_NOT_NULL(listener) );
}

// Removes a property listener.
void PropertyListenerCollection::RemovePropertyListener(PropertyListener *pListener)
{
	m_listeners.remove(pListener);
}

// Calls all property listeners.
void PropertyListenerCollection::EmitPropertyChanged(const PropertyProvider &provider) const
{
	for (listener_list::const_iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
		(*it)->PropertyChanged(provider);
}

namespace
{

struct PropertyTransfer : public PropertyVisitor
{
	PropertyProvider *dest;
	uint4 destID;

	PropertyTransfer(PropertyProvider &dest, uint4 id)
		: dest(&dest), destID(id) { }

	void Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, const void *values)
	{
		dest->SetProperty(destID, desc.TypeInfo->type, values, desc.Count);
	}
};

} // namespace

// Transfers all from the given source property provider to the given destination property provider.
void TransferProperties(PropertyProvider &dest, const PropertyProvider &source)
{
	const uint4 count = dest.GetPropertyCount();
	const uint4 srcCount = source.GetPropertyCount();

	uint4 nextID = 0;

	for (uint4 srcID = 0; srcID < srcCount; ++srcID)
	{
		utf8_ntr srcName = source.GetPropertyName(srcID);

		uint4 lowerID = nextID;
		uint4 upperID = nextID;

		for (uint4 i = 0; i < count; ++i)
		{
			// Perform bi-directional search: even == forward; odd == backward
			uint4 id = (lean::is_odd(i) | (upperID == count)) & (lowerID != 0)
				? --lowerID
				: upperID++;

			if (dest.GetPropertyName(id) == srcName)
			{
				PropertyTransfer transfer(dest, id);
				source.ReadProperty(srcID, transfer);

				// Start next search with next property
				nextID = id + 1;
				break;
			}
		}
	}
}

} // namespace
