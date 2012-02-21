/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_DEPENDENCIES_IMPL
#define BE_CORE_DEPENDENCIES_IMPL

#include "beCore.h"
#include "beDependencies.h"
#include <vector>
#include <lean/smart/weak_resource_ptr.h>
#include <list>
#include <lean/concurrent/critical_section.h>
#include <lean/functional/algorithm.h>
#include <utility>

namespace beCore
{

/// Dependency.
template <class Value>
class DependencyImpl : public Dependency<Value>
{
public:
	/// Value type.
	typedef Value Value;
	/// Dependent interface.
	typedef beCore::Dependent<Value> Dependent;

private:
	lean::critical_section *m_pCriticalSection;

	Value m_value;

	typedef std::vector< lean::weak_resource_ptr<Dependent> > dependent_vector; 
	dependent_vector m_asyncDependents;
	dependent_vector m_syncDependents;

	/// Notifies the given dependents about dependency changes. 
	void NotifiyDependents(dependent_vector &dependents, const Value &newValue) const
	{
		typename dependent_vector::iterator it = dependents.begin();
		typename dependent_vector::iterator itCleanedEnd = dependents.end();

		while (it != itCleanedEnd)
		{
			lean::resource_ptr<Dependent> pDependent = *it;

			if (pDependent)
			{
				pDependent->DependencyChanged(m_value, newValue);
				++it;
			}
			else
				// Clean while notifying
				*it = *--itCleanedEnd;
		}

		dependents.erase(itCleanedEnd, dependents.end());
	}

public:
	/// Constructor.
	DependencyImpl(const Value &value, lean::critical_section *pCriticalSection)
		: m_value(value),
		m_pCriticalSection( LEAN_ASSERT_NOT_NULL(pCriticalSection) ) { }

	/// Adds a synchronous dependent.
	void AddDependentSync(Dependent *pDependent)
	{
		lean::scoped_cs_lock lock(*m_pCriticalSection);
		m_syncDependents.push_back( LEAN_ASSERT_NOT_NULL(pDependent) );
	}
	/// Adds an asynchronous dependent.
	void AddDependentASync(Dependent *pDependent)
	{
		lean::scoped_cs_lock lock(*m_pCriticalSection);
		m_asyncDependents.push_back( LEAN_ASSERT_NOT_NULL(pDependent) );
	}
	/// Removes a dependent.
	void RemoveDependent(Dependent *pDependent)
	{
		lean::scoped_cs_lock lock(*m_pCriticalSection);
		lean::remove(m_syncDependents, pDependent);
		lean::remove(m_asyncDependents, pDependent);
	}

	/// Notifies synchronous dependents about dependency changes.
	void NotifySyncDependents(const Value &newValue)
	{
		lean::scoped_cs_lock lock(*m_pCriticalSection);
		NotifiyDependents(m_syncDependents, newValue);
	}
	/// Notifies asynchronous dependents about dependency changes.
	void NotifyAsyncDependents(const Value &newValue)
	{
		lean::scoped_cs_lock lock(*m_pCriticalSection);
		NotifiyDependents(m_asyncDependents, newValue);
	}

	/// Checks if this dependency currently has synchronous dependents.
	bool HasSyncDependents() const
	{
		return !m_syncDependents.empty();
	}
};

namespace Detail
{
	template <class Ref, class Value>
	struct PointerComparison
	{
		Ref *ref;

		LEAN_INLINE bool operator ()(const Value &value) const
		{
			return lean::addressof(value) == ref;
		}

		LEAN_INLINE PointerComparison(Ref *ref)
			: ref(ref) { }
	};
}

/// Dependency manager.
template <class Value>
class DependenciesImpl : public Dependencies<Value>
{
public:
	/// Value type.
	typedef Value Value;
	/// Dependency class.
	typedef beCore::Dependency<Value> Dependency;
	/// Dependency class.
	typedef beCore::DependencyImpl<Value> DependencyImpl;

private:
	lean::critical_section m_criticalSection;

	typedef std::list<DependencyImpl> dependency_list;
	dependency_list m_dependencies;

	typedef std::pair<Dependency*, Value> dependency_pair;
	typedef std::vector<dependency_pair> dependency_vector;
	dependency_vector m_changedDependencies;

public:
	/// Constructor.
	DependenciesImpl() { };
	
	/// Adds a dependency to this dependency manager.
	Dependency* AddDependency(const Value &value)
	{
		lean::scoped_cs_lock lock(m_criticalSection);
		m_dependencies.push_back( DependencyImpl(value, &m_criticalSection) );
		return &m_dependencies.back();
	}
	/// Removes a dependency from this dependency manager.
	void RemoveDependency(Dependency *pDependency)
	{
		lean::scoped_cs_lock lock(m_criticalSection);

		// WARNING: Invalidate pending dependency changes before removing dependency
		for (dependency_vector::iterator it = m_changedDependencies.begin(); it != m_changedDependencies.end(); ++it)
			if (it->first == pDependency)
				it->first = nullptr;

		std::remove_if(m_dependencies.begin(), m_dependencies.end(), Detail::PointerComparison<Dependency, DependencyImpl>(pDependency) );
	}

	/// Notifies dependents about changes in the given dependency.
	void DependencyChanged(Dependency *pDependency, const Value &newValue)
	{
		lean::scoped_cs_lock lock(m_criticalSection);

		pDependency->NotifyAsyncDependents(newValue);

		if (pDependency->HasSyncDependents())
			m_changedDependencies.push_back( dependency_pair(pDependency, newValue) );
	}

	/// Notifies synchrounous dependents about pending dependency changes.
	/// Returns true iff new dependency changes have been generated during notfication.
	bool NotifiySyncDependents()
	{
		lean::scoped_cs_lock lock(m_criticalSection);

		const size_t changedDependencyCount = m_changedDependencies.size();

		// WARNING: Vector may change anytime
		// -> Use INDICES instead of iterators
		for (size_t n = 0; n < changedDependencyCount; ++n)
		{
			const dependency_pair &dependency = m_changedDependencies[n];

			// -> Check for INVALIDATED dependencies
			if (dependency.first)
				dependency.first->NotifySyncDependents(dependency.second);
		}

		m_changedDependencies.erase(m_changedDependencies.begin(),
			m_changedDependencies.begin() + changedDependencyCount);

		return HasPendingSyncNotifications();
	}
	/// Notifies synchrounous dependents about pending dependency changes until there are no new changes pending.
	void NotifiyAllSyncDependents()
	{
		while (NotifiySyncDependents());
	}

	/// Checks if there are synchronous dependency change notifications pending.
	bool HasPendingSyncNotifications() const
	{
		return !m_changedDependencies.empty(); 
	}
};

} // namespace

#endif