// berc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "berc.h"
#include <lean/logging/log.h>
#include <lean/logging/log_stream.h>
#include <fstream>
#include <lean/io/file.h>
#include <map>
#include <string>

/// Registered command line tools.
typedef std::map<std::string, const CommandLineTool*> tool_map;
tool_map g_tools;

bool g_fromBatch = false;

// Runs a specified command line tool.
int main(int argc, const char* argv[])
{
	lean::log_stream coutLogStream(&std::cout);
	lean::error_log().add_target(&coutLogStream);
	lean::info_log().add_target(&coutLogStream);

	// Find requested tool
	tool_map::const_iterator itTool = g_tools.find((argc > 1) ? argv[1] : "help");
	int toolArgOffset = min(2, argc);
	
	if (itTool == g_tools.end())
		itTool = g_tools.find("help");

	// ASSERT: help always present
	LEAN_ASSERT(itTool != g_tools.end());
	
	// Don't store commands in batch mode
	if (argc > toolArgOffset && stricmp(argv[toolArgOffset], "batch") == 0)
	{
		g_fromBatch = true;
		++toolArgOffset;
	}

	try
	{
		return itTool->second->Run(argc - toolArgOffset, &argv[toolArgOffset]);
	}
	catch (const std::runtime_error &error)
	{
		std::cout << "ERROR: An exception occurred: " << error.what() << std::endl;
		return -1;
	}
}

// Registers the given command line tool.
void RegisterTool(const char *name, const CommandLineTool *pTool)
{
	LEAN_ASSERT(name);
	LEAN_ASSERT(pTool);

	g_tools[name] = pTool;
}

// Unregisters the given command line tool.
void UnregisterTool(const char *name)
{
	g_tools.erase(name);
}

// Stores the current command for the given file.
void StoreCommand(const char *tool, const char *file, const char *const *args, size_t argCount)
{
	std::string batFilename;
	batFilename.reserve(strlen(file) + strlen(tool) + lean::ntarraylen("..berc.bat"));
	batFilename.append(file);
	batFilename.append(".");
	batFilename.append(tool);
	batFilename.append(".berc.bat");

	bool bWriteBat = true;

	if (g_fromBatch)
	{
		try
		{
			lean::file batFile(batFilename, lean::file::readwrite, lean::file::append);
			// Bat file exists, touch and don't modify
			bWriteBat = false;
			batFile.touch();
		}
		catch (...) { }
	}

	if (bWriteBat)
	{
		std::ofstream batFile(batFilename, std::ios::out | std::ios::trunc);
		batFile << "@pushd \"%~dp0\"\r\n";
		batFile << LEAN_QUOTE_VALUE(BERC_COMMAND) " " << tool << " batch";
	
		for (size_t i = 0; i < argCount; ++i)
		{
			bool bQuote = (strchr(args[i], ' ') != nullptr);

			batFile << ' ';
			if (bQuote)
				batFile << '"';
			batFile << args[i];
			if (bQuote)
				batFile << '"';
		}

		batFile << "\r\n@popd";
	}
}

/// Help tool.
const struct HelpTool : public CommandLineTool
{
	/// Constructor.
	HelpTool() { RegisterTool("help", this); }
	/// Destructor.
	~HelpTool() { UnregisterTool("help"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		std::cout << "****************************************************************" << std::endl;
		std::cout << " breeze Resource Compiler                  (c) Tobias Zirr 2011 " << std::endl;
		std::cout << "****************************************************************" << std::endl << std::endl;

		std::cout << " Syntax: berc <tool> <args ...>"  << std::endl << std::endl;

		std::cout << " Tools:"  << std::endl;

		for (tool_map::const_iterator it = g_tools.begin(); it != g_tools.end(); ++it)
			std::cout << " -> " << it->first << std::endl;

		std::cout << std::endl;
		
		std::cout << "Press ENTER to exit ...";
		std::cin.get();

		return 0;
	}

} g_helpTool;
