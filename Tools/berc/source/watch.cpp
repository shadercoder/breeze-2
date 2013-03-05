// berc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "berc.h"

#include <Windows.h>
#include <Shellapi.h>
#include <lean/smart/handle_guard.h>

#include <vector>
#include <string>

#include <lean/logging/win_errors.h>
#include <lean/io/numeric.h>
#include <lean/io/filesystem.h>

/// Watch tool help.
const struct WatchToolHelp : public CommandLineTool
{
	/// Constructor.
	WatchToolHelp() { RegisterTool("watchhelp", this); }
	/// Destructor.
	~WatchToolHelp() { UnregisterTool("watchhelp"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		std::cout << " Syntax: berc watch [/r] [/ext] <dir>"  << std::endl << std::endl;

		std::cout << " Arguments:"  << std::endl;
		std::cout << "  /r             Recursively"  << std::endl;
		std::cout << "  /ext           Batch file extension mask. Default: '*.berc.bat'"  << std::endl;
		std::cout << "  <dir>          Directory to observe"  << std::endl;

		return 0;
	}

} g_watchToolHelp;

struct find_close_handle_policy
{
	static LEAN_INLINE HANDLE invalid() { return NULL; }
	static LEAN_INLINE void release(HANDLE handle) { ::FindClose(handle); }
};

/// Watch tool.
const struct WatchTool : public CommandLineTool
{
	/// Constructor.
	WatchTool() { RegisterTool("watch", this); }
	/// Destructor.
	~WatchTool() { UnregisterTool("watch"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		if (argc < 1)
		{
			g_watchToolHelp.Run(argc, argv);
			return -1;
		}

		const char *directory = argv[argc - 1];
		lean::utf8_ntr extension( "*.berc.bat" );
		bool recursive = false;

		for (int i = 0; i < argc - 1; ++i)
		{
			const char *arg = argv[i];

			if (_strnicmp(arg, "/r", lean::ntarraylen("/r")) == 0)
				recursive = true;
			else if (_strnicmp(arg, "/ext:", lean::ntarraylen("/ext:")) == 0)
				extension = lean::utf8_ntr( &arg[lean::ntarraylen("/ext:")] );
			else
				std::cout << "Unrecognized argument, consult watchhelp for help: " << arg << std::endl;
		}

		lean::handle_guard<::HANDLE> hDirectory(
				::CreateFileW(
						lean::utf_to_utf16(directory).c_str(),
						FILE_GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
						nullptr,
						OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS ,
						0
					)
			);
		if (hDirectory == INVALID_HANDLE_VALUE)
		{
			LEAN_LOG_WIN_ERROR_CTX("CreateFileW()", "Unable to observe directory");
			std::cout << directory << std::endl;
			return -1;
		}

		std::wstring prevFile;

		while (true)
		{
			static const DWORD bufferSize = 4096;
			union
			{
				DWORD dwBuffer[bufferSize];
				char buffer[sizeof(DWORD[bufferSize])];
			};
			DWORD bytesRead;
			BOOL bSucceeded = ::ReadDirectoryChangesW(
					hDirectory,
					dwBuffer, bufferSize,
					recursive,
					FILE_NOTIFY_CHANGE_LAST_WRITE,
					&bytesRead, nullptr, nullptr
				);
			if (!bSucceeded)
				LEAN_LOG_WIN_ERROR_CTX("ReadDirectoryChangesW()", "Waiting on file changed notifications");
			else
			{
				// MONITOR: HACK: TODO: Ugly way to avoid write conflicts ...
				Sleep(500);

				for (size_t bufferOffset = 0, bufferSkip = -1; bufferSkip != 0; bufferOffset += bufferSkip)
				{
					// Get changed file
					const FILE_NOTIFY_INFORMATION& info = *reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(buffer + bufferOffset);
					std::wstring notificationFile(info.FileName, info.FileNameLength / sizeof(WCHAR));
					bufferSkip = info.NextEntryOffset;

					// HACK: Filter redundant notifications ...
					if (notificationFile == prevFile)
						continue;
					prevFile = notificationFile;
					
					// Build find mask
					lean::utf8_string file = lean::utf_to_utf8(notificationFile);
					lean::utf8_string dir = lean::get_directory<lean::utf8_string>(file) + '\\';
					file.reserve(file.size() + 1 + extension.size());
					file.append(1, '.');
					file.append(extension.begin(), extension.end());

					std::cout << "Change detected, looking for: " << file << std::endl;

					// Find corresponding batch files
					WIN32_FIND_DATAW findData;
					lean::handle_guard<::HANDLE> hFindBat(
							::FindFirstFileW( lean::utf_to_utf16(file).c_str(), &findData )
						);

					if (hFindBat != INVALID_HANDLE_VALUE)
					{
						do
						{
							lean::utf8_string batFile = dir + lean::utf_to_utf8(findData.cFileName);
							std::cout << std::endl << "RUNNING: " << batFile << std::endl;

							// TODO: Compare dates?

							if ((int) ::ShellExecuteW(nullptr,  nullptr, lean::utf_to_utf16(batFile).c_str(), nullptr, nullptr, SW_HIDE) <= 32)
								LEAN_LOG_WIN_ERROR_MSG( "ShellExecuteW()");

							std::cout << std::endl;
						}
						while (::FindNextFileW(hFindBat, &findData));
					}
				}
			}
		}

		return 0;
	}

} g_watchTool;
