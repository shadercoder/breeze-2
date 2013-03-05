/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#pragma once
#ifndef BE_RESOURCECOMPILER_SERIALIZATION
#define BE_RESOURCECOMPILER_SERIALIZATION

#include "beResourceCompiler.h"

namespace beResourceCompiler
{

/// Mesh writing flags.
struct MeshWriteFlags
{
	// Enumeration.
	enum T
	{
		Normals = 1 << 0,			///< Include vertex normals.
		Colors = 1 << 1,			///< Include vertex colors.
		TexCoords = 1 << 2,			///< Include texture coordinates.
		Tangents = 1 << 3,			///< Include tangents.
		BiTangents = 1 << 4,		///< Include bi-tangents.

		TangentFrame = Tangents | BiTangents,	///< Include tangent frame.

		WideIndices = 1 << 16,		///< Write wide indices.

		SubsetNames = 1 << 17		///< Include subset names.
	};
	LEAN_MAKE_ENUM_STRUCT(MeshWriteFlags)
};

// Prototypes
class Mesh;
class Scene;
class PhysicsCooker;

/// Saves the given mesh to the given file.
BE_RESOURCECOMPILER_API void SaveMesh(const utf8_ntri &file, const Mesh &mesh,
	uint4 flags = MeshWriteFlags::Normals | MeshWriteFlags::TexCoords | MeshWriteFlags::SubsetNames);

/// Saves a physical representation of the given mesh to the given file.
BE_RESOURCECOMPILER_API void SavePhysXShapes(const utf8_ntri &file, const Scene &scene, PhysicsCooker &cooker);

}

#endif