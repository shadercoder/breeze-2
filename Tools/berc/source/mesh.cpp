// berc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "berc.h"
#include <beResourceCompiler/beMeshImporter.h>
#include <beResourceCompiler/beMesh.h>
#include <beResourceCompiler/beMeshSerialization.h>

#include <vector>
#include <string>

#include <lean/io/numeric.h>
#include <lean/io/filesystem.h>

#include <lean/smart/resource_ptr.h>

/// Mesh tool help.
const struct MeshToolHelp : public CommandLineTool
{
	/// Constructor.
	MeshToolHelp() { RegisterTool("meshhelp", this); }
	/// Destructor.
	~MeshToolHelp() { UnregisterTool("meshhelp"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		std::cout << " Syntax: berc mesh [/VDn] [/Vc] [/VDt] [/Vtan] [/Vbtan] [/Vsn] [/Vsna] [/Von] [/Tsf] [/Iw] [/O] [/S]  [/Ms] <input> <output>"  << std::endl << std::endl;

		std::cout << " Arguments:"  << std::endl;
		std::cout << "  /VDn           Don't include vertex normals"  << std::endl;
		std::cout << "  /Vc            Include vertex colors"  << std::endl;
		std::cout << "  /VDt           Don't include vertex tex coords"  << std::endl;
		std::cout << "  /VFt           Enforce vertex tex coords"  << std::endl;
		std::cout << "  /Vtan          Include vertex tangents"  << std::endl;
		std::cout << "  /Vbtan         Include vertex bi-tangents"  << std::endl;
		std::cout << "  /Vsn           Re-generate smoothed normals"  << std::endl;
		std::cout << "  /Vsna:<float>  Set maximum smoothing angle to <float> degrees (default 30°)"  << std::endl;
		std::cout << "  /Von           Generate object normals"  << std::endl;
		std::cout << "  /Tsf:<float>   Set scale factor to <float> (default 1.0)"  << std::endl;
		std::cout << "  /Iw            Write wide indices"  << std::endl;
		std::cout << "  /O             Optimize mesh"  << std::endl;
		std::cout << "  /S             Sort meshes"  << std::endl;
		std::cout << "  /Ms            Single material"  << std::endl;
		std::cout << "  <input>        Input mesh file path"  << std::endl;
		std::cout << "  <output>       Output mesh file path"  << std::endl;

		return 0;
	}

} g_meshToolHelp;

/// Mesh tool.
const struct MeshTool : public CommandLineTool
{
	/// Constructor.
	MeshTool() { RegisterTool("mesh", this); }
	/// Destructor.
	~MeshTool() { UnregisterTool("mesh"); }

	/// Runs the command line tool.
	int Run(int argc, const char* argv[]) const
	{
		if (argc < 2)
		{
			g_meshToolHelp.Run(argc, argv);
			return -1;
		}

		std::vector<const char*> storedArgs;
		storedArgs.reserve(argc);

		const char *inputFile = argv[argc - 2];
		const char *outputFile = argv[argc - 1];

		uint4 importerFlags = beResourceCompiler::MeshLoadFlags::Normals | beResourceCompiler::MeshLoadFlags::TexCoords;
		uint4 writeFlags = beResourceCompiler::MeshWriteFlags::Normals | beResourceCompiler::MeshWriteFlags::TexCoords
			| beResourceCompiler::MeshWriteFlags::SubsetNames;
		float smoothingAngle = 30.0f;
		float scaleFactor = 1.0f;

		for (int i = 0; i < argc - 2; ++i)
		{
			const char *arg = argv[i];

			if (_stricmp(arg, "/VDn") == 0)
			{
				// Always import (-> tangent space)
				writeFlags &= ~beResourceCompiler::MeshWriteFlags::Normals;
			}
			else if (_stricmp(arg, "/Vc") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::Colors;
				writeFlags |= beResourceCompiler::MeshWriteFlags::Colors;
			}
			else if (_stricmp(arg, "/VDt") == 0)
			{
				// Always import (-> tangent space)
				writeFlags &= ~beResourceCompiler::MeshWriteFlags::TexCoords;
			}
			else if (_stricmp(arg, "/VFt") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::ForceUV;
			}
			else if (_stricmp(arg, "/Vtan") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::Tangents;
				writeFlags |= beResourceCompiler::MeshWriteFlags::Tangents;
			}
			else if (_stricmp(arg, "/Vbtan") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::Tangents;
				writeFlags |= beResourceCompiler::MeshWriteFlags::BiTangents;
			}
			else if (_stricmp(arg, "/Vsn") == 0)
			{
				// Writing controlled by /VDn
				importerFlags |= beResourceCompiler::MeshLoadFlags::SmoothNormals;
			}
			else if (_strnicmp(arg, "/Vsna:", lean::ntarraylen("/Vsna:")) == 0)
			{
				lean::string_to_float(
					&arg[lean::ntarraylen("/Vsna:")],
					smoothingAngle);
			}
			else if (_stricmp(arg, "/Von") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::ObjectNormals;
				writeFlags |= beResourceCompiler::MeshWriteFlags::BiTangents;
			}
			else if (_strnicmp(arg, "/Tsf:", lean::ntarraylen("/Tsf:")) == 0)
			{
				lean::string_to_float(
					&arg[lean::ntarraylen("/Tsf:")],
					scaleFactor);
			}
			else if (_stricmp(arg, "/Iw") == 0)
			{
				writeFlags |= beResourceCompiler::MeshWriteFlags::WideIndices;
			}
			else if (_stricmp(arg, "/O") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::Optimize;
			}
			else if (_stricmp(arg, "/S") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::Sort;
			}
			else if (_stricmp(arg, "/Ms") == 0)
			{
				importerFlags |= beResourceCompiler::MeshLoadFlags::RemoveMaterials;
			}
			else
				std::cout << "Unrecognized argument, consult meshhelp for help: " << arg << std::endl;

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

			StoreCommand("mesh", inputFile, storedArgs.data(), storedArgs.size());
		}

		beResourceCompiler::MeshImporter importer;
		lean::resource_ptr<beResourceCompiler::Mesh> pMesh = importer.LoadMesh(inputFile, importerFlags, smoothingAngle, scaleFactor);
		beResourceCompiler::SaveMesh(outputFile, *pMesh, writeFlags);

		return 0;
	}

} g_meshTool;
