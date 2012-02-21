/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_FILEWATCH
#define BE_CORE_FILEWATCH

#include "beCore.h"
#include <lean/tags/noncopyable.h>
#include <lean/pimpl/pimpl_ptr.h>
#include <lean/strings/types.h>

namespace beCore
{

/// Interface providing methods allowing for the observation of file changes.
class FileObserver
{
public:
	/// Method called whenever an observed file has changed.
	virtual void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision) = 0;
};

/// File watch class that allows for the observation of file changes.
class FileWatch : public lean::noncopyable
{
private:
	class Impl;
	friend class Impl;
	lean::pimpl_ptr<Impl> m_impl;

public:
	/// Constructor.
	BE_CORE_API FileWatch();
	/// Destructor.
	BE_CORE_API ~FileWatch();

	/// Adds the given observer to be called when the given file has been modified.
	BE_CORE_API bool AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Adds the given observer, iff it hasn't been added yet.
	BE_CORE_API bool AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Removes the given observer no longer to be called when the given file is modified.
	BE_CORE_API void RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
};

/// Gets the file watch.
BE_CORE_API FileWatch& GetFileWatch();

} // namespace

#endif