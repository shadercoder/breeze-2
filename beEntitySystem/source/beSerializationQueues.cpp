/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beSerializationJob.h"
#include "beEntitySystem/beSerializationTasks.h"

#include <boost/ptr_container/ptr_list.hpp>

namespace beEntitySystem
{

// Gets generic tasks to perform when saving.
SaveQueue& GetWorldSaveTasks()
{
	static SaveQueue saveTasks;
	return saveTasks;
}

// Gets generic tasks to perform when loading.
LoadQueue& GetWorldLoadTasks()
{
	static LoadQueue loadTasks;
	return loadTasks;
}

// Gets generic tasks to perform when saving.
SaveQueue& GetResourceSaveTasks()
{
	static SaveQueue saveTasks;
	return saveTasks;
}

// Gets generic tasks to perform when loading.
LoadQueue& GetResourceLoadTasks()
{
	static LoadQueue loadTasks;
	return loadTasks;
}

/// Implementation.
struct SaveQueue::M
{
	// NOTE: ptr_list template does not support const pointers?
	typedef boost::ptr_sequence_adapter< const SaveJob, std::list<const void*> > job_list;
	job_list jobs;
};

// Constructor.
SaveQueue::SaveQueue()
	: m( new M() )
{
}

// Destructor.
SaveQueue::~SaveQueue()
{
}

// Takes ownership of the given serialization job.
void SaveQueue::AddSerializationJob(const SaveJob *pJob)
{
	m->jobs.push_back( LEAN_ASSERT_NOT_NULL(pJob) );
}

// Saves anything, e.g. to the given XML root node.
void SaveQueue::Save(rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters)
{
	// Execute all jobs
	for (M::job_list::const_iterator it = m->jobs.begin(); it != m->jobs.end(); ++it)
		// NOTE: New jobs might be added any time
		it->Save(root, parameters, *this);
}

/// Implementation.
struct LoadQueue::M
{
	// NOTE: ptr_list template does not support const pointers?
	typedef boost::ptr_sequence_adapter< const LoadJob, std::list<const void*> > job_list;
	job_list jobs;
};

// Constructor.
LoadQueue::LoadQueue()
	: m( new M() )
{
}

// Destructor.
LoadQueue::~LoadQueue()
{
}

// Takes ownership of the given serialization job.
void LoadQueue::AddSerializationJob(const LoadJob *pJob)
{
	m->jobs.push_back( LEAN_ASSERT_NOT_NULL(pJob) );
}

// Loads anything, e.g. from the given XML root node.
void LoadQueue::Load(const rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters)
{
	// Execute all jobs
	for (M::job_list::const_iterator it = m->jobs.begin(); it != m->jobs.end(); ++it)
		// NOTE: New jobs might be added any time
		it->Load(root, parameters, *this);
}

} // namespace
