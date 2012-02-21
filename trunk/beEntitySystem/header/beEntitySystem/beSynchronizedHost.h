/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SYNCHRONIZEDHOST
#define BE_ENTITYSYSTEM_SYNCHRONIZEDHOST

#include "beEntitySystem.h"
#include "beSynchronized.h"
#include <vector>

namespace beEntitySystem
{

/// Synchronized host clas.
class SynchronizedHost : public Synchronized
{
private:
	typedef std::vector<Synchronized*> synchronized_vector;
	synchronized_vector m_synchFlush;
	synchronized_vector m_synchFetch;

public:
	/// Constructor.
	BE_ENTITYSYSTEM_API SynchronizedHost();
	/// Destructor.
	BE_ENTITYSYSTEM_API ~SynchronizedHost();

	/// Synchronizes synchronized objects with the simulation.
	BE_ENTITYSYSTEM_API void Flush();
	/// Synchronizes the simulation with synchronized objects.
	BE_ENTITYSYSTEM_API void Fetch();

	/// Adds a synchronized controller.
	BE_ENTITYSYSTEM_API void AddSynchronized(Synchronized *pSynchronized, uint4 flags);
	/// Removes a synchronized controller.
	BE_ENTITYSYSTEM_API void RemoveSynchronized(Synchronized *pSynchronized, uint4 flags);
};

} // namespace

#endif