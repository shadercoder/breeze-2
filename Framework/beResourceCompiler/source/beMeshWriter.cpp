/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/beMeshSerialization.h"
#include "beResourceCompiler/beMeshImpl.h"
#include <beScene/beMeshSerialization.h>
#include <lean/io/raw_file.h>
#include <lean/meta/strip.h>

#include <lean/logging/log.h>

namespace beResourceCompiler
{

namespace
{

/// Writs the given data to the given file.
template <class Type>
uint4 WriteData(lean::raw_file &file, const typename lean::identity<Type>::type &data)
{
	return static_cast<uint4>( file.write( reinterpret_cast<const char*>(&data), sizeof(data) ) );
}

struct ChunkInfo
{
	uint8 SizeOffset;
	uint4 Size;

	ChunkInfo(uint8 sizeOffset = -1)
		: SizeOffset(sizeOffset),
		Size(0) { }
};

/// Writs the given data to the given file.
uint4 BeginChunk(lean::raw_file &file, beScene::MeshDataChunk::T chunkID, ChunkInfo &info)
{
	info.SizeOffset = file.pos() + offsetof(beScene::MeshDataChunkHeader, ChunkSize);
	info.Size = 0;
	return WriteData<beScene::MeshDataChunkHeader>( file, beScene::MeshDataChunkHeader(chunkID, 0) );
}

/// Completes the given chunk.
uint4 EndChunk(lean::raw_file &file, ChunkInfo &info)
{
	uint8 currentPos = file.pos();
	file.pos(info.SizeOffset);
	
	WriteData<uint4>(file, info.Size);
	
	file.pos(currentPos);
	return 0;
}

/// Writes a data chunk.
template <class Type>
uint4 WriteDataChunk(lean::raw_file &file, beScene::MeshDataChunk::T chunkID, const typename lean::identity<Type>::type &data)
{
	uint4 size = WriteData<beScene::MeshDataChunkHeader>(
			file,
			beScene::MeshDataChunkHeader( chunkID, static_cast<uint4>(sizeof(data)) )
		);
	size += static_cast<uint4>( file.write(reinterpret_cast<const char*>(&data), sizeof(data)) );
	return size;
}

/// Writes a string chunk.
uint4 WriteStringChunk(lean::raw_file &file, beScene::MeshDataChunk::T chunkID,
	const char *text, uint4 length)
{
	uint4 size = WriteData<beScene::MeshDataChunkHeader>( file, beScene::MeshDataChunkHeader(chunkID, length) );
	size += static_cast<uint4>( file.write(text, length) );
	return size;
}

/// Gets the vertex attributes provided by the given mesh, masked by flags.
uint4 GetVertexAttributes(const aiMesh &mesh, uint4 flags)
{
	uint4 vertexAttributes = 0;

	if (mesh.HasPositions())
		vertexAttributes |= beScene::MeshVertexAttributes::Position;
	
	if (mesh.HasNormals() && (flags & MeshWriteFlags::Normals))
		vertexAttributes |= beScene::MeshVertexAttributes::Normal;
	
	if (mesh.HasVertexColors(0) && (flags & MeshWriteFlags::Colors))
		vertexAttributes |= beScene::MeshVertexAttributes::Color;

	if (mesh.HasTextureCoords(0) && (flags & MeshWriteFlags::TexCoords))
		vertexAttributes |= beScene::MeshVertexAttributes::TexCoord;

	if (mesh.HasTangentsAndBitangents() && (flags & MeshWriteFlags::Tangents))
		vertexAttributes |= beScene::MeshVertexAttributes::Tangent;
	if (mesh.HasTangentsAndBitangents() && (flags & MeshWriteFlags::BiTangents))
		vertexAttributes |= beScene::MeshVertexAttributes::BiTangent;

	return vertexAttributes;
}

/// Writes the given vertices.
uint4 WriteVertexDescChunk(lean::raw_file &file, uint4 vertexAttributes)
{
	uint4 metaSize = 0;

	ChunkInfo descChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Desc, descChunk);

	if (vertexAttributes & beScene::MeshVertexAttributes::Position)
		descChunk.Size += WriteData<beScene::MeshDataVertexElementDesc>(
				file,
				beScene::MeshDataVertexElementDesc(beScene::MeshVertexAttributes::Position, 0, beGraphics::Format::R32G32B32F)
			);

	if (vertexAttributes & beScene::MeshVertexAttributes::Normal)
		descChunk.Size += WriteData<beScene::MeshDataVertexElementDesc>(
				file,
				beScene::MeshDataVertexElementDesc(beScene::MeshVertexAttributes::Normal, 0, beGraphics::Format::R32G32B32F)
			);

	if (vertexAttributes & beScene::MeshVertexAttributes::Color)
		descChunk.Size += WriteData<beScene::MeshDataVertexElementDesc>(
				file,
				beScene::MeshDataVertexElementDesc(beScene::MeshVertexAttributes::Color, 0, beGraphics::Format::R8G8B8A8U)
			);

	if (vertexAttributes & beScene::MeshVertexAttributes::TexCoord)
		descChunk.Size += WriteData<beScene::MeshDataVertexElementDesc>(
				file,
				beScene::MeshDataVertexElementDesc(beScene::MeshVertexAttributes::TexCoord, 0, beGraphics::Format::R32G32F)
			);

	if (vertexAttributes & beScene::MeshVertexAttributes::Tangent)
		descChunk.Size += WriteData<beScene::MeshDataVertexElementDesc>(
				file,
				beScene::MeshDataVertexElementDesc(beScene::MeshVertexAttributes::Tangent, 0, beGraphics::Format::R32G32B32F)
			);
	if (vertexAttributes & beScene::MeshVertexAttributes::BiTangent)
		descChunk.Size += WriteData<beScene::MeshDataVertexElementDesc>(
				file,
				beScene::MeshDataVertexElementDesc(beScene::MeshVertexAttributes::BiTangent, 0, beGraphics::Format::R32G32B32F)
			);

	metaSize += EndChunk(file, descChunk);
	return metaSize + descChunk.Size;
}

/// Writes the given vertices.
uint4 WriteVertexChunk(lean::raw_file &file, const aiMesh &mesh, uint4 vertexAttributes)
{
	uint4 metaSize = 0;

	ChunkInfo vertexChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Data, vertexChunk);

	for (uint4 i = 0; i < mesh.mNumVertices; ++i)
	{
		if (vertexAttributes & beScene::MeshVertexAttributes::Position)
			vertexChunk.Size += WriteData<aiVector3D>(file, mesh.mVertices[i]);

		if (vertexAttributes & beScene::MeshVertexAttributes::Normal)
			vertexChunk.Size += WriteData<aiVector3D>(file, mesh.mNormals[i]);

		if (vertexAttributes & beScene::MeshVertexAttributes::Color)
		{
			const aiColor4D &color = mesh.mColors[0][i];

			uint4 colorValue = ((uint4) (color.a * 255) << 24)
				| ((uint4) (color.r * 255) << 16)
				| ((uint4) (color.g * 255) << 8)
				| ((uint4) (color.b * 255) << 0);

			vertexChunk.Size += WriteData<uint4>(file, colorValue);
		}

		if (vertexAttributes & beScene::MeshVertexAttributes::TexCoord)
		{
			vertexChunk.Size += WriteData<float>(file, mesh.mTextureCoords[0][i].x);
			vertexChunk.Size += WriteData<float>(file, mesh.mTextureCoords[0][i].y);
		}

		if (vertexAttributes & beScene::MeshVertexAttributes::Tangent)
			vertexChunk.Size += WriteData<aiVector3D>(file, mesh.mTangents[i]);
		if (vertexAttributes & beScene::MeshVertexAttributes::BiTangent)
			vertexChunk.Size += WriteData<aiVector3D>(file, mesh.mBitangents[i]);
	}

	metaSize += EndChunk(file, vertexChunk);
	return metaSize + vertexChunk.Size;
}

/// Saves the vertices of the given mesh.
uint4 SaveVertices(lean::raw_file &file, const aiMesh &mesh, uint4 flags)
{
	uint4 metaSize = 0;

	ChunkInfo verticesChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Vertices, verticesChunk);

	uint4 vertexAttributes = GetVertexAttributes(mesh, flags);

	// Count
	{
		verticesChunk.Size += WriteDataChunk<uint4>(file, beScene::MeshDataChunk::Count, mesh.mNumVertices);
	}

	// Description
	{
		verticesChunk.Size += WriteVertexDescChunk(file, vertexAttributes);
	}

	// Data
	{
		verticesChunk.Size += WriteVertexChunk(file, mesh, vertexAttributes);
	}

	metaSize += EndChunk(file, verticesChunk);
	return metaSize + verticesChunk.Size;
}

/// Writes the indices of the given faces.
uint4 WriteIndexChunk(lean::raw_file &file, const aiFace *faces, const aiFace *facesEnd, uint4 flags)
{
	uint4 metaSize = 0;

	ChunkInfo indexChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Data, indexChunk);

	if (flags & MeshWriteFlags::WideIndices)
	{
		for (const aiFace *face = faces; face < facesEnd; ++face)
			for (int i = 0; i < 3; ++i)
				indexChunk.Size += WriteData<uint4>( file, face->mIndices[i] );
	}
	else
	{
		for (const aiFace *face = faces; face < facesEnd; ++face)
			for (int i = 0; i < 3; ++i)
				indexChunk.Size += WriteData<uint2>( file, static_cast<uint2>(face->mIndices[i]) );
	}

	metaSize += EndChunk(file, indexChunk);
	return metaSize + indexChunk.Size;
}

/// Saves the indices of the given mesh.
uint4 SaveIndices(lean::raw_file &file, const aiMesh &mesh, uint4 flags)
{
	uint4 metaSize = 0;

	ChunkInfo indicesChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Indices, indicesChunk);

	LEAN_ASSERT(mesh.mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

	// Count
	{
		indicesChunk.Size += WriteDataChunk<uint4>(file, beScene::MeshDataChunk::Count, mesh.mNumFaces * 3);
	}

	// Description
	{
		beGraphics::Format::T format = (flags & MeshWriteFlags::WideIndices) ? beGraphics::Format::R32U : beGraphics::Format::R16U;
		indicesChunk.Size += WriteDataChunk<uint4>(file, beScene::MeshDataChunk::Desc, format);
	}

	// Data
	{
		indicesChunk.Size += WriteIndexChunk(file, mesh.mFaces, mesh.mFaces + mesh.mNumFaces, flags);
	}

	metaSize += EndChunk(file, indicesChunk);
	return metaSize + indicesChunk.Size;
}

/// Saves the given mesh.
uint4 SaveSubset(lean::raw_file &file, const aiMesh &mesh, const aiScene &scene, uint4 flags)
{
	uint4 metaSize = 0;

	ChunkInfo subsetChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Data, subsetChunk);

	aiString materialName;
	scene.mMaterials[mesh.mMaterialIndex]->Get(AI_MATKEY_NAME, materialName);
	const aiString &meshName = (mesh.mName.length != 0) ? mesh.mName : materialName;

	LEAN_LOG("Writing subset \"" << meshName.data << "\" (Material: \"" << materialName.data << "\")");

	// Mesh name
	if (flags & MeshWriteFlags::SubsetNames)
	{
		if (meshName.length > 0)
			subsetChunk.Size += WriteStringChunk(file, beScene::MeshDataChunk::Name, meshName.data, static_cast<uint4>(meshName.length));
	}
	
	// Vertices & indices
	{
		// Enforce wide indices, if required
		if (mesh.mNumVertices > static_cast<uint2>(-1))
			flags |= MeshWriteFlags::WideIndices;

		subsetChunk.Size += SaveVertices(file, mesh, flags);
		subsetChunk.Size += SaveIndices(file, mesh, flags);
	}

	metaSize += EndChunk(file, subsetChunk);
	return metaSize + subsetChunk.Size;
}

/// Saves all subsets in the given node.
uint4 SaveSubsets(lean::raw_file &file, const aiScene &scene, uint4 flags)
{
	uint4 metaSize = 0;

	ChunkInfo subsetsChunk;
	metaSize += BeginChunk(file, beScene::MeshDataChunk::Subsets, subsetsChunk);

	// Subset count
	{
		subsetsChunk.Size += WriteDataChunk<uint4>(file, beScene::MeshDataChunk::Count, scene.mNumMeshes);
	}

	// Subsets
	{
		for (uint4 i = 0; i < scene.mNumMeshes; ++i)
			subsetsChunk.Size += SaveSubset(file, *scene.mMeshes[i], scene, flags);
	}

	metaSize += EndChunk(file, subsetsChunk);
	return metaSize + subsetsChunk.Size;
}

} // namespace

// Saves the given mesh to the given file.
void SaveMesh(const utf8_ntri &file, const Mesh &mesh, uint4 flags)
{
	const MeshImpl &meshImpl = static_cast<const MeshImpl&>(mesh);

	lean::raw_file meshFile(file, lean::file::readwrite, lean::file::overwrite);

	// Mesh
	ChunkInfo meshChunk;
	BeginChunk(meshFile, beScene::MeshDataChunk::Header, meshChunk);
	
	// Mesh name
	{
		const aiNode &rootNode = *meshImpl.GetScene()->mRootNode;

		if (rootNode.mName.length > 0)
			meshChunk.Size += WriteStringChunk(meshFile, beScene::MeshDataChunk::Name, rootNode.mName.data, static_cast<uint4>(rootNode.mName.length));
	}

	// Mesh subsets
	{
		meshChunk.Size += SaveSubsets(meshFile, *meshImpl.GetScene(), flags);
	}

	EndChunk(meshFile, meshChunk);
}

} // namespace
