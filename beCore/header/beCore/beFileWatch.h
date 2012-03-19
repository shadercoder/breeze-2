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
	/// Called whenever an observed file has changed.
	virtual void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision) = 0;
};

/// Interface providing methods allowing for the observation of directory changes.
class DirectoryObserver
{
public:
	/// Called whenever an observed directory has changed.
	virtual void DirectoryChanged(const lean::utf8_ntri &directory) = 0;
};

/// File watch class that allows for the observation of file changes.
class FileWatch : public lean::noncopyable
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_CORE_API FileWatch();
	/// Destructor.
	BE_CORE_API ~FileWatch();

	/// Adds the given observer to be called when the given file is modified.
	BE_CORE_API bool AddObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Adds the given observer, iff it hasn't been added yet.
	BE_CORE_API bool AddOrKeepObserver(const lean::utf8_ntri &file, FileObserver *pObserver);
	/// Removes the given observer, no longer to be called when the given file is modified.
	BE_CORE_API void RemoveObserver(const lean::utf8_ntri &file, FileObserver *pObserver);

	/// Adds the given observer to be called when the given directory is modified.
	BE_CORE_API bool AddObserver(const lean::utf8_ntri &directory, DirectoryObserver *pObserver);
	/// Removes the given observer, no longer to be called when the given directory is modified.
	BE_CORE_API void RemoveObserver(const lean::utf8_ntri &directory, DirectoryObserver *pObserver);
};

/// Gets the file watch.
BE_CORE_API FileWatch& GetFileWatch();

} // namespace

#endif