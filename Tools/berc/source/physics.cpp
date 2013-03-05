// berc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "berc.h"
#include <beResourceCompiler/bePhysicsCooker.h>
#include <beResourceCompiler/beMeshImporter.h>
#include <beResourceCompiler/beScene.h>
#include <beResourceCompiler/beMeshSerialization.h>
#include <lean/smart/resource_ptr.h>

#include <vector>
#include <string>

#include <lean/io/numeric.h>
#include <lean/io/filesystem.h>

/// Physics tool help.
const struct PhysicsToolHelp : public CommandLineTool
{
	/// Constructor.
	PhysicsToolHelp() { RegisterTool("physicshelp", this); }
	/// Destructor.
	~PhysicsToolHelp() { UnregisterTool("physicshelp"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		std::cout << " Syntax: berc physics [/Tsf] <input> <output>"  << std::endl << std::endl;

		std::cout << " Arguments:"  << std::endl;
		std::cout << "  /Tsf:<float>   Set scale factor to <float> (default 1.0)"  << std::endl;
		std::cout << "  <input>        Input mesh file path"  << std::endl;
		std::cout << "  <output>       Output mesh file path"  << std::endl;

		return 0;
	}

} g_physicsToolHelp;

/// Physics tool.
const struct PhysicsTool : public CommandLineTool
{
	/// Constructor.
	PhysicsTool() { RegisterTool("physics", this); }
	/// Destructor.
	~PhysicsTool() { UnregisterTool("physics"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		if (argc < 2)
		{
			g_physicsToolHelp.Run(argc, argv);
			return -1;
		}

		std::vector<const char*> storedArgs;
		storedArgs.reserve(argc);

		const char *inputFile = argv[argc - 2];
		const char *outputFile = argv[argc - 1];
		float scaleFactor = 1.0f;

		for (int i = 0; i < argc - 2; ++i)
		{
			const char *arg = argv[i];

			if (_strnicmp(arg, "/Tsf:", lean::ntarraylen("/Tsf:")) == 0)
			{
				lean::string_to_float(
					&arg[lean::ntarraylen("/Tsf:")],
					scaleFactor);
			}
			else
				std::cout << "Unrecognized argument, consult physicshelp for help: " << arg << std::endl;

			storedArgs.push_back(arg);
		}

		{
			std::string inputFilename = lean::get_filename(inputFile);
			storedArgs.push_back(inputFilename.c_str());
			std::string relativeOutputFile = lean::relative_path<std::string>(
					lean::absolute_path(outputFile),
					lean::get_directory<std::string>( lean::absolute_path(inputFile) )
				);
			storedArgs.push_back(relativeOutputFile.c_str());

			StoreCommand("physics", inputFile, storedArgs.data(), storedArgs.size());
		}

		beResourceCompiler::MeshImporter importer;
		lean::resource_ptr<beResourceCompiler::Scene> pScene = importer.LoadScene(inputFile, 0, 30.0f, scaleFactor);

		beResourceCompiler::PhysicsCooker cooker;
		beResourceCompiler::SavePhysXShapes(outputFile, *pScene, cooker);

		return 0;
	}

} g_physicsTool;
