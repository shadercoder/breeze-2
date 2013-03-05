#ifndef BERC_HEADER
#define BERC_HEADER

#include <cstddef>

/// Command line tool interface.
class CommandLineTool
{
public:
	virtual ~CommandLineTool() { }

	/// Runs the command line tool.
	virtual int Run(int argc, const char* argv[]) const = 0;
};

/// Registers the given command line tool.
void RegisterTool(const char *name, const CommandLineTool *pTool);
/// Unregisters the given command line tool.
void UnregisterTool(const char *name);

/// Stores the current command for the given file.
void StoreCommand(const char *tool, const char *file, const char *const *args, size_t argCount);

#endif