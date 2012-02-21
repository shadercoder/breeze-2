/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/beFileWatch.h"

#include <unordered_map>
#include <boost/ptr_container/ptr_map_adapter.hpp>

#include <lean/tags/noncopyable.h>
#include <lean/logging/log.h>
#include <lean/logging/win_errors.h>
#include <lean/io/filesystem.h>
#include <lean/smart/handle_guard.h>

#include <lean/concurrent/thread.h>
#include <lean/functional/callable.h>

#include <lean/concurrent/critical_section.h>
#include <lean/concurrent/event.h>

/// Implementation of the file system class internals.
class beCore::FileWatch::Impl
{
private:
	/// Directory.
	class Directory;
	/// Filesystem directory.
	class FileSystemDirectory;

	typedef boost::ptr_map_adapter< Directory, std::unordered_map<lean::utf8_string, void*> > directory_map;
	directory_map m_directories;

	lean::critical_section m_directoryLock;

	volatile bool m_bShuttingDown;
	lean::event m_updateEvent;

	lean::thread m_observationThread;

	/// Observes files & directories.
	void ObservationThread();

public:
	/// Constructor.
	Impl();
	/// Destructor.
	~Impl();

	/// Adds the given observer to be called when the given file has been modified.
	bool AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Adds the given observer, iff it hasn't been added yet.
	bool AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Removes the given observer no longer to be called when the given file is modified.
	void RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
};

/// Directory.
class beCore::FileWatch::Impl::Directory : public lean::tags::noncopyable
{
private:
	/// Observer
	struct ObserverInfo
	{
		lean::utf8_string file;
		lean::uint8 lastRevision;
		FileObserver *pObserver;

		ObserverInfo(const lean::utf8_ntri &file,
			lean::uint8 revision,
			FileObserver *pObserver)
				: file(file.to<lean::utf8_string>()),
				lastRevision(revision),
				pObserver(pObserver) { }
	};
	typedef std::unordered_multimap<lean::utf8_string, ObserverInfo> observer_map;
	observer_map m_observers;

	lean::critical_section m_observerLock;

protected:
	/// Gets the revision of the given file.
	virtual lean::uint8 GetRevision(const lean::utf8_ntri &file) const = 0;

public:
	/// Constructor.
	Directory();
	/// Destructor.
	virtual ~Directory();

	/// Adds the given observer to be called when the given file has been modified.
	bool AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Adds the given observer, iff it hasn't been added yet.
	bool AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Removes the given observer no longer to be called when the given file is modified.
	void RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver);

	/// Notifies file observers about modifications to the files they are observing. 
	virtual void FilesChanged();

	/// Gets the handle to a change notification event or NULL, if unavailable.
	virtual HANDLE GetChangeNotification() const = 0;
};

// Filesystem directory.
class beCore::FileWatch::Impl::FileSystemDirectory : public beCore::FileWatch::Impl::Directory
{
private:
	struct close_change_notification_handle_policy
	{
		static LEAN_INLINE HANDLE invalid() { return NULL; }
		static LEAN_INLINE void release(HANDLE handle) { ::FindCloseChangeNotification(handle); }
	};
	lean::handle_guard<HANDLE, close_change_notification_handle_policy> m_hChangeNotification;

	/// Creates a file change notification handle for the given directory.
	static HANDLE CreateFileChangeNotification(const lean::utf8_ntri &directory);

protected:
	/// Gets the revision of the given file.
	virtual lean::uint8 GetRevision(const lean::utf8_ntri &file) const;

public:
	/// Constructor.
	FileSystemDirectory(const lean::utf8_ntri &directory);
	/// Destructor.
	virtual ~FileSystemDirectory();

	// Notifies file observers about modifications to the files they are observing. 
	virtual void FilesChanged();

	/// Gets the handle to a change notification event or NULL, if unavailable.
	virtual HANDLE GetChangeNotification() const;
};

// Constructor.
beCore::FileWatch::FileWatch()
	: m_impl(new Impl())
{
}

// Destructor.
beCore::FileWatch::~FileWatch()
{
}

// Adds the given observer to be called when the given file has been modified.
bool beCore::FileWatch::AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	return m_impl->AddObserver(file, pObserver);
}
// Adds the given observer, iff it hasn't been added yet.
bool beCore::FileWatch::AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	return m_impl->AddOrKeepObserver(file, pObserver);
}
// Removes the given observer no longer to be called when the given file is modified.
void beCore::FileWatch::RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	m_impl->RemoveObserver(file, pObserver);
}

// Gets the file watch.
beCore::FileWatch& beCore::GetFileWatch()
{
	static FileWatch instance;
	return instance;
}

// Constructor.
LEAN_INLINE beCore::FileWatch::Impl::Impl()
	: m_bShuttingDown(false),
	m_observationThread( lean::make_callable(this, &Impl::ObservationThread) )
{
}
// Destructor.
LEAN_INLINE beCore::FileWatch::Impl::~Impl()
{
	// ORDER: Signal update AFTER all modifications have been completed
	m_bShuttingDown = true;
	m_updateEvent.set();

	// Wait for observation thread to exit gracefully
	m_observationThread.join();
}

// Adds the given observer to be called when the given file has been modified.
LEAN_INLINE bool beCore::FileWatch::Impl::AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	lean::utf8_string canonicalFile = lean::absolute_path<lean::utf8_string>(file);
	lean::utf8_string directory = lean::get_directory<lean::utf8_string>(canonicalFile);

	directory_map::iterator itDirectory = m_directories.find(directory);

	if (itDirectory == m_directories.end())
	{
		// Don't modify directory map while being accessed from the observer thread
		lean::scoped_cs_lock lock(m_directoryLock);

		itDirectory = m_directories.insert(directory, new FileSystemDirectory(directory) ).first;

		// Notify observation thread about new directory
		// ORDER: Signal update AFTER all modifications have been completed
		m_updateEvent.set();
	}

	return itDirectory->second->AddObserver(canonicalFile, pObserver);
}

// Adds the given observer, iff it hasn't been added yet.
LEAN_INLINE bool beCore::FileWatch::Impl::AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	lean::utf8_string canonicalFile = lean::absolute_path<lean::utf8_string>(file);
	lean::utf8_string directory = lean::get_directory<lean::utf8_string>(canonicalFile);

	directory_map::iterator itDirectory = m_directories.find(directory);

	// Either delegate to directory or add directory + observer if neither existent yet
	return (itDirectory == m_directories.end())
		? AddObserver(file, pObserver)
		: itDirectory->second->AddOrKeepObserver(canonicalFile, pObserver);
}

// Removes the given observer no longer to be called when the given file is modified.
LEAN_INLINE void beCore::FileWatch::Impl::RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	lean::utf8_string canonicalFile = lean::absolute_path<lean::utf8_string>(file);
	lean::utf8_string directory = lean::get_directory<lean::utf8_string>(canonicalFile);

	directory_map::iterator itDirectory = m_directories.find(directory);

	if (itDirectory != m_directories.end())
		itDirectory->second->RemoveObserver(canonicalFile, pObserver);
}

// Observes files & directories.
void beCore::FileWatch::Impl::ObservationThread()
{
	try
	{
		std::vector<Directory*> directories;
		std::vector<HANDLE> events;

		while (!m_bShuttingDown)
		{
			events.clear();
			directories.clear();

			{
				// Don't modify directory map while accessed from this thread
				lean::scoped_cs_lock lock(m_directoryLock);

				const size_t directoryCount = m_directories.size();

				directories.reserve(directoryCount + 1);
				events.reserve(directoryCount + 1);

				// Interrupt waiting when there are updates outside this thread
				directories.push_back(nullptr);
				events.push_back(m_updateEvent.native_handle());

				const directory_map::iterator itDirectoryEnd = m_directories.end();

				for (directory_map::iterator itDirectory = m_directories.begin();
					itDirectory != itDirectoryEnd; ++itDirectory)
				{
					Directory *pDirectory = itDirectory->second;
					HANDLE hChangeNotification = pDirectory->GetChangeNotification();

					if (hChangeNotification != NULL)
					{
						// Store changed notification event handles along with their corresponding directory instances
						directories.push_back(pDirectory);
						events.push_back(hChangeNotification);
					}
				}
			}

			DWORD dwSignal = ::WaitForMultipleObjects(static_cast<DWORD>(events.size()), &events[0], false, INFINITE);

			if (dwSignal == WAIT_FAILED)
				LEAN_LOG_WIN_ERROR_CTX("WaitForMultipleObjects()", "Waiting on file changed notifications");

			// Exclude update event (index 0)
			if (WAIT_OBJECT_0 < dwSignal && dwSignal < WAIT_OBJECT_0 + events.size())
			{
				LEAN_ASSERT(events.size() == directories.size());
			
				// Get the directory that triggered the changed notification event
				Directory *pDirectory = directories[dwSignal - WAIT_OBJECT_0];

				LEAN_ASSERT(pDirectory);

				pDirectory->FilesChanged();
			}

			// Updating now
			m_updateEvent.reset();
		}
	}
	catch (const std::exception &exc)
	{
		LEAN_LOG_ERROR_CTX(exc.what(), "File change observation thread");
	}
	catch (...)
	{
		LEAN_LOG_ERROR_MSG("Unhandled exception in file change observation thread.");
	}
}

// Constructor.
beCore::FileWatch::Impl::Directory::Directory()
{
}

// Destructor.
beCore::FileWatch::Impl::Directory::~Directory()
{
}

// Adds the given observer to be called when the given file has been modified.
bool beCore::FileWatch::Impl::Directory::AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	if (!pObserver)
	{
		LEAN_LOG_ERROR_MSG("pObserver == nullptr");
		return false;
	}

	// Do not modify observers while processing changed notifications
	lean::scoped_cs_lock lock(m_observerLock);

	m_observers.insert( observer_map::value_type(
		lean::get_filename<lean::utf8_string>(file),
		ObserverInfo(file, GetRevision(file), pObserver) ) );

	return true;
}

// Adds the given observer, iff it hasn't been added yet.
bool beCore::FileWatch::Impl::Directory::AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	lean::utf8_string fileName = lean::get_filename<lean::utf8_string>(file);

	// Do not modify observers while processing changed notifications
	lean::scoped_cs_lock lock(m_observerLock);

	const observer_map::iterator itObserverInfoEnd = m_observers.upper_bound(fileName);

	for (observer_map::iterator itObserverInfo = m_observers.lower_bound(fileName);
		itObserverInfo != itObserverInfoEnd; ++itObserverInfo)
		// Keep matching observer
		if (itObserverInfo->second.pObserver == pObserver)
			return true; 

	// Add observer, if not found
	return AddObserver(file, pObserver);
}

// Removes the given observer no longer to be called when the given file is modified.
void beCore::FileWatch::Impl::Directory::RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver)
{
	lean::utf8_string fileName = lean::get_filename<lean::utf8_string>(file);

	// Do not modify observers while processing changed notifications
	lean::scoped_cs_lock lock(m_observerLock);

	const observer_map::iterator itObserverInfoEnd = m_observers.upper_bound(fileName);

	for (observer_map::iterator itObserverInfo = m_observers.lower_bound(fileName);
		itObserverInfo != itObserverInfoEnd; ++itObserverInfo)
		// Erase ALL matching observation entries
		if (itObserverInfo->second.pObserver == pObserver)
			m_observers.erase(itObserverInfo);
}

// Notifies file observers about modifications to the files they are observing. 
void beCore::FileWatch::Impl::Directory::FilesChanged()
{
	// Do not modify observers while processing changed notifications
	lean::scoped_cs_lock lock(m_observerLock);

	const observer_map::iterator itObserverInfoEnd = m_observers.end();

	for (observer_map::iterator itObserverInfo = m_observers.begin();
		itObserverInfo != itObserverInfoEnd; ++itObserverInfo)
	{
		ObserverInfo &info = itObserverInfo->second;

		// Source of revision depends on type of directory
		lean::uint8 currentRevision = GetRevision(info.file);

		// Notify about updates AND revertions
		// (custom filtering may be performed via the revision argument passed)
		if (currentRevision != info.lastRevision)
		{
			info.lastRevision = currentRevision;

			try
			{
				info.pObserver->FileChanged(info.file, currentRevision);
			}
			catch (const std::exception &exc)
			{
				LEAN_LOG_ERROR_CTX(exc.what(), info.file.c_str());
			}
			catch (...)
			{
				LEAN_LOG_ERROR_CTX("Unhandled exception on file observer notificatio.", info.file.c_str());
			}
		}
	}
}

// Creates a file change notification handle for the given directory.
HANDLE beCore::FileWatch::Impl::FileSystemDirectory::CreateFileChangeNotification(const lean::utf8_ntri &directory)
{
	HANDLE hChangeNotification = ::FindFirstChangeNotificationW(
		lean::utf_to_utf16(directory).c_str(),
		FALSE,
		FILE_NOTIFY_CHANGE_LAST_WRITE);

	if (hChangeNotification == INVALID_HANDLE_VALUE || hChangeNotification == (HANDLE)ERROR_INVALID_FUNCTION)
	{
		LEAN_LOG_WIN_ERROR_CTX("FindFirstChangeNotification()", directory.c_str());
		// Convert to generic error value
		hChangeNotification = NULL;
	}
	
	return hChangeNotification;
}

// Constructor.
beCore::FileWatch::Impl::FileSystemDirectory::FileSystemDirectory(const lean::utf8_ntri &directory)
	: m_hChangeNotification( CreateFileChangeNotification(directory) )
{
}

// Destructor.
beCore::FileWatch::Impl::FileSystemDirectory::~FileSystemDirectory()
{
}

// Gets the revision of the given file.
lean::uint8 beCore::FileWatch::Impl::FileSystemDirectory::GetRevision(const lean::utf8_ntri &file) const
{
	return lean::file_revision(file);
}

// Gets the handle to a change notification event or NULL, if unavailable.
HANDLE beCore::FileWatch::Impl::FileSystemDirectory::GetChangeNotification() const
{
	return m_hChangeNotification;
}

// Notifies file observers about modifications to the files they are observing. 
void beCore::FileWatch::Impl::FileSystemDirectory::FilesChanged()
{
	if (m_hChangeNotification != NULL)
	{
		if (!::FindNextChangeNotification(m_hChangeNotification))
			LEAN_LOG_WIN_ERROR_CTX("FindNextChangeNotification()", "???");
	}

	Directory::FilesChanged();
}
