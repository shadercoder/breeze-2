/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beSynchronizedHost.h"
#include <lean/functional/algorithm.h>
#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Constructor.
SynchronizedHost::SynchronizedHost()
{
}

// Destructor.
SynchronizedHost::~SynchronizedHost()
{
}

// Synchronizes synchronized objects with the simulation.
void SynchronizedHost::Flush()
{
	for (synchronized_vector::const_iterator it = m_synchFlush.begin(); it != m_synchFlush.end(); ++it)
		(*it)->Flush();
}

// Synchronizes the simulation with synchronized objects.
void SynchronizedHost::Fetch()
{
	for (synchronized_vector::const_iterator it = m_synchFetch.begin(); it != m_synchFetch.end(); ++it)
		(*it)->Fetch();
}

// Adds a synchronized controller.
void SynchronizedHost::AddSynchronized(Synchronized *pSynchronized, uint4 flags)
{
	if (!pSynchronized)
	{
		LEAN_LOG_ERROR_MSG("pSynchronized may not be nullptr");
		return;
	}

	if (flags & SynchronizedFlags::Flush)
		m_synchFlush.push_back(pSynchronized);
	if (flags & SynchronizedFlags::Fetch)
		m_synchFetch.push_back(pSynchronized);
}

// Removes a synchronized controller.
void SynchronizedHost::RemoveSynchronized(Synchronized *pSynchronized, uint4 flags)
{
	if (flags & SynchronizedFlags::Flush)
		lean::remove(m_synchFlush, pSynchronized);
	if (flags & SynchronizedFlags::Fetch)
		lean::remove(m_synchFetch, pSynchronized);
}

} // namespace
