/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#ifndef BE_RESOURCECOMPILER_MESHIMPORTER
#define BE_RESOURCECOMPILER_MESHIMPORTER

#include "beResourceCompiler.h"
#include "beMesh.h"
#include "beScene.h"
#include <lean/smart/scoped_ptr.h>
#include <lean/smart/resource_ptr.h>

namespace beResourceCompiler
{

/// Mesh loading flags.
namespace MeshLoadFlags
{
	// Enumeration.
	enum T
	{
		Normals = 1 << 0,			/// Include vertex normals.
		Colors = 1 << 1,			/// Include vertex colors.
		TexCoords = 1 << 2,			/// Include texture coordinates.
		Tangents = 1 << 3,			/// Include tangent frame.

		SmoothNormals = 1 << 4,		/// Regenerate all normals.
		
		NonIndexed = 1 << 5			/// Disables indexing (required for physics processing).
	};
}

/// Mesh importer.
class MeshImporter
{
private:
	struct Data;
	lean::scoped_ptr<Data> m_data;

public:
	/// Constructor.
	BE_RESOURCECOMPILER_API MeshImporter();
	/// Destructor.
	BE_RESOURCECOMPILER_API ~MeshImporter();

	/// Reads a mesh from the given file.
	BE_RESOURCECOMPILER_API lean::resource_ptr<Mesh, true> LoadMesh(const utf8_ntri &file,
		uint4 flags = MeshLoadFlags::Normals | MeshLoadFlags::TexCoords,
		float smoothingAngle = 30.0f, float scaleFactor = 1.0f);
	/// Reads a scene from the given file.
	BE_RESOURCECOMPILER_API lean::resource_ptr<Scene, true> LoadScene(const utf8_ntri &file,
		uint4 flags = MeshLoadFlags::Normals | MeshLoadFlags::TexCoords,
		float smoothingAngle = 30.0f, float scaleFactor = 1.0f);
};

}

#endif