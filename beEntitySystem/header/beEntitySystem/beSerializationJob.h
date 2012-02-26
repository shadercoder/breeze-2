/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SERIALIZATION_JOB
#define BE_ENTITYSYSTEM_SERIALIZATION_JOB

#include "beEntitySystem.h"
#include <lean/tags/noncopyable.h>
#include <beCore/beShared.h>
#include <lean/rapidxml/rapidxml.hpp>
#include <lean/pimpl/pimpl_ptr.h>

namespace beCore
{
	class ParameterSet;
}

namespace beEntitySystem
{

/// Serialization queue.
template <class Job>
class LEAN_INTERFACE SerializationQueue
{
protected:
	SerializationQueue& operator =(const SerializationQueue&) { return *this; };
	~SerializationQueue() { }

public:
	/// Takes ownership of the given serialization job.
	virtual void AddSerializationJob(const Job *pJob) = 0;
};

/// Serialization job interface.
class LEAN_INTERFACE SaveJob : public lean::noncopyable_chain<beCore::Shared>
{
protected:
	SaveJob& operator =(const SaveJob&) { return *this; };

public:
	/// Destructor.
	virtual ~SaveJob() { }
	
	/// Saves anything, e.g. to the given XML root node.
	virtual void Save(rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, SerializationQueue<SaveJob> &queue) const = 0;
};

/// Serialization job interface.
class LEAN_INTERFACE LoadJob : public lean::noncopyable_chain<beCore::Shared>
{
protected:
	LoadJob& operator =(const LoadJob&) { return *this; };

public:
	/// Destructor.
	virtual ~LoadJob() { }
	
	/// Loads anything, e.g from the given XML root node.
	virtual void Load(const rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters, SerializationQueue<LoadJob> &queue) const = 0;
};

/// Serialization queue.
class SaveQueue : public lean::noncopyable, public SerializationQueue<SaveJob>
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_ENTITYSYSTEM_API SaveQueue();
	/// Destructor.
	BE_ENTITYSYSTEM_API ~SaveQueue();

	/// Takes ownership of the given serialization job.
	BE_ENTITYSYSTEM_API void AddSerializationJob(const SaveJob *pJob);

	/// Saves anything, e.g. to the given XML root node.
	BE_ENTITYSYSTEM_API void Save(rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters);
};

/// Serialization queue.
class LoadQueue : public lean::noncopyable, public SerializationQueue<LoadJob>
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_ENTITYSYSTEM_API LoadQueue();
	/// Destructor.
	BE_ENTITYSYSTEM_API ~LoadQueue();

	/// Takes ownership of the given serialization job.
	BE_ENTITYSYSTEM_API void AddSerializationJob(const LoadJob *pJob);

	/// Loads anything, e.g from the given XML root node.
	BE_ENTITYSYSTEM_API void Load(const rapidxml::xml_node<lean::utf8_t> &root, beCore::ParameterSet &parameters);
};

/// Instantiate this to add a serializer of the given type to the global queue of save tasks.
template <class SaveTask>
struct SaveTaskPlugin
{
	/// Adds a global save task of the given type.
	SaveTaskPlugin()
	{
		GetSaveTasks().AddSerializationJob( new SaveTask() );
	}
};

/// Instantiate this to add a loader of the given type to the global queue of load tasks.
template <class LoadTask>
struct LoadTaskPlugin
{
	/// Adds a global load task of the given type.
	LoadTaskPlugin()
	{
		GetLoadTasks().AddSerializationJob( new LoadTask() );
	}
};

} // namespace

#endif