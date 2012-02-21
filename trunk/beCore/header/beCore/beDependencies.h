/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_DEPENDENCIES
#define BE_CORE_DEPENDENCIES

#include "beCore.h"
#include "beShared.h"
#include <lean/tags/noncopyable.h>

namespace beCore
{

/// Dependenct interface.
template <class Value>
class Dependent : public beCore::ResourceInterface
{
public:
	/// Value type.
	typedef Value Value;

protected:
	Dependent& operator =(const Dependent&) { return *this; }

public:
	virtual ~Dependent() throw() { }

	/// Notifies the dependent about dependency changes.
	virtual void DependencyChanged(const Value &oldValue, const Value &newValue) = 0;
};

/// Dependency interface.
template <class Value>
class Dependency
{
public:
	/// Value type.
	typedef Value Value;
	/// Dependent interface.
	typedef Dependent<Value> Dependent;

protected:
	Dependency& operator =(const Dependency&) { return *this; }
	~Dependency() throw() { }

public:
	/// Adds a synchronous dependent.
	virtual void AddDependentSync(Dependent *pDependent) = 0;
	/// Adds an asynchronous dependent.
	virtual void AddDependentASync(Dependent *pDependent) = 0;
	/// Removes a dependent.
	virtual void RemoveDependent(Dependent *pDependent) = 0;

	/// Notifies synchronous dependents about dependency changes.
	virtual void NotifySyncDependents(const Value &newValue) = 0;
	/// Notifies asynchronous dependents about dependency changes.
	virtual void NotifyAsyncDependents(const Value &newValue) = 0;
	/// Checks if this dependency currently has synchronous dependents.
	virtual bool HasSyncDependents() const = 0;
};

/// Dependency manager.
template <class Value>
class Dependencies : public lean::noncopyable, public beCore::Resource
{
public:
	/// Value type.
	typedef Value Value;
	/// Dependency class.
	typedef Dependency<Value> Dependency;

public:
	/// Adds a dependency to this dependency manager.
	virtual Dependency* AddDependency(const Value &value) = 0;
	/// Removes a dependency from this dependency manager.
	virtual void RemoveDependency(Dependency *pDependency) = 0;

	/// Notifies dependents about changes in the given dependency.
	virtual void DependencyChanged(Dependency *pDependency, const Value &value) = 0;

	/// Notifies synchrounous dependents about pending dependency changes.
	/// Returns true iff new dependency changes have been generated during notfication.
	virtual bool NotifiySyncDependents() = 0;
	/// Notifies synchrounous dependents about pending dependency changes until there are no new changes pending.
	virtual void NotifiyAllSyncDependents() = 0;
	/// Checks if there are synchronous dependency change notifications pending.
	virtual bool HasPendingSyncNotifications() const = 0;
};

} // namespace

#endif