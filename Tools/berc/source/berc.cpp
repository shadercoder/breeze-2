// berc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "berc.h"
#include <lean/logging/log.h>
#include <lean/logging/log_stream.h>
#include <map>

/// Registered command line tools.
typedef std::map<std::string, const CommandLineTool*> tool_map;
tool_map g_tools;

// Runs a specified command line tool.
int main(int argc, const char* argv[])
{
	tool_map::const_iterator itTool = g_tools.find((argc > 1) ? argv[1] : "help");

	if (itTool == g_tools.end())
		itTool = g_tools.find("help");

	// ASSERT: help always present
	LEAN_ASSERT(itTool != g_tools.end());

	lean::log_stream coutLogStream(&std::cout);
	lean::error_log().add_target(&coutLogStream);
	lean::info_log().add_target(&coutLogStream);

	try
	{
		return itTool->second->Run(argc - 2, &argv[2]);
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
