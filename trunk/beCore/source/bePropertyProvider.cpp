/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/bePropertyProvider.h"

#include "beCore/bePropertyListener.h"

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

} // namespace
