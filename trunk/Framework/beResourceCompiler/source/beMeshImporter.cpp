/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/beMeshImporter.h"
#include "beResourceCompiler/beMeshImpl.h"
#include "beResourceCompiler/beSceneImpl.h"

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>

namespace beResourceCompiler
{

/// Implementation data.
struct MeshImporter::Data
{
	Assimp::Importer importer;
};

// Constructor.
MeshImporter::MeshImporter()
	: m_data( new Data() )
{
}

// Destructor.
MeshImporter::~MeshImporter()
{
}

namespace
{

/// Sets the importer up from the given flags.
uint4 SetUpImporter(Assimp::Importer &importer, const uint4 flags, float smoothingAngle)
{
	// DirectX space
	uint4 assimpFlags = aiProcess_ConvertToLeftHanded;

	// Polygons only
	assimpFlags |= aiProcess_FindDegenerates | aiProcess_SortByPType;
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

	// Triangles only
	assimpFlags |= aiProcess_Triangulate;

	if (~flags & MeshLoadFlags::NonIndexed)
		assimpFlags |= aiProcess_JoinIdenticalVertices;

	// UVs only
	assimpFlags |= aiProcess_GenUVCoords | aiProcess_TransformUVCoords;

	// Reduce mesh & material count
	assimpFlags |= aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials;

	// Complete mesh with normals
	if (flags & MeshLoadFlags::Normals)
	{
		assimpFlags |= aiProcess_GenSmoothNormals;
		importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, smoothingAngle);
	}

	// Complete mesh with tangents
	if (flags & MeshLoadFlags::Tangents)
	{
		assimpFlags |= aiProcess_CalcTangentSpace;
	}

	// Complete with object normals
	if (flags & MeshLoadFlags::ObjectNormals)
	{
		assimpFlags |= aiProcess_GenObjectNormals;
		LEAN_LOG("Object normals enabled, this might take a while.");
	}

	// Optimize mesh
	if (flags & MeshLoadFlags::Optimize)
	{
		assimpFlags |= aiProcess_ImproveCacheLocality;
		importer.SetPropertyInteger(AI_CONFIG_PP_ICL_PTCACHE_SIZE, 64);
		LEAN_LOG("Mesh optimization enabled, this might take a while.");
	}

	uint4 assimpRemoveFlags = 0;

	// Remove existing normals, if regeneration requested
	if (~flags & MeshLoadFlags::Normals || flags & MeshLoadFlags::SmoothNormals)
		assimpRemoveFlags |= aiComponent_NORMALS;
	if (~flags & MeshLoadFlags::Colors)
		assimpRemoveFlags |= aiComponent_COLORS;
	if (~flags & MeshLoadFlags::TexCoords)
		assimpRemoveFlags |= aiComponent_TEXCOORDS;
	// Remove existing tangent space, if regeneration requested
	if (~flags & MeshLoadFlags::Tangents || flags & MeshLoadFlags::SmoothNormals)
		assimpRemoveFlags |= aiComponent_TANGENTS_AND_BITANGENTS;

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, assimpRemoveFlags);
	if (assimpRemoveFlags != 0)
		assimpFlags |= aiProcess_RemoveComponent;

	return assimpFlags;
}

/// Reads the given scene file, applies the given scaling & post-processing steps.
const aiScene* LoadScaleAndProcess(Assimp::Importer &importer, const utf8_ntri &file, uint4 assimpFlags, const uint4 loadFlags, float scaleFactor)
{
	const aiScene *pScene = importer.ReadFile(file.c_str(), 0);

	if (!pScene)
		LEAN_THROW_ERROR_XCTX( importer.GetErrorString(), "Assimp Loading", file.c_str() );

	if (scaleFactor != 1.0f)
	{
		aiMatrix4x4 scaling;
		aiMatrix4x4::Scaling( aiVector3D(scaleFactor), scaling );
		const_cast<aiMatrix4x4&>(pScene->mRootNode->mTransformation) = scaling * pScene->mRootNode->mTransformation;
	}

	if (loadFlags & MeshLoadFlags::RemoveMaterials)
	{
		const uint4 meshCount = pScene->mNumMeshes;

		for (uint4 i = 0; i < meshCount; ++i)
			pScene->mMeshes[i]->mMaterialIndex = 0;
	}

	if (loadFlags & MeshLoadFlags::ForceUV)
	{
		const uint4 materialCount = pScene->mNumMaterials;

		for (uint4 i = 0; i < materialCount; ++i)
		{
			int mapping = aiTextureMapping_BOX;
			pScene->mMaterials[i]->Get(AI_MATKEY_MAPPING_DIFFUSE(0), mapping);
			pScene->mMaterials[i]->AddProperty(&mapping, 1, AI_MATKEY_MAPPING_DIFFUSE(0));
		}
	}

	pScene = importer.ApplyPostProcessing(assimpFlags);

	if (!pScene)
		LEAN_THROW_ERROR_XCTX( importer.GetErrorString(), "Assimp Post-processing", file.c_str() );

	return pScene;
}

} // namespace

// Reads a mesh from the given file.
lean::resource_ptr<Mesh, true> MeshImporter::LoadMesh(const utf8_ntri &file, const uint4 flags, float smoothingAngle, float scaleFactor)
{
	uint4 assimpFlags = SetUpImporter(m_data->importer, flags, smoothingAngle);

	// Flatten scene
	assimpFlags |= aiProcess_PreTransformVertices;

	return lean::bind_resource<Mesh>(
			new MeshImpl( LoadScaleAndProcess(m_data->importer, file.c_str(), assimpFlags, flags, scaleFactor) )
		);
}

// Reads a scene from the given file.
lean::resource_ptr<Scene, true> MeshImporter::LoadScene(const utf8_ntri &file, const uint4 flags, float smoothingAngle, float scaleFactor)
{
	uint4 assimpFlags = SetUpImporter(m_data->importer, flags, smoothingAngle);

	return lean::bind_resource<Scene>(
			new SceneImpl( LoadScaleAndProcess(m_data->importer, file.c_str(), assimpFlags, flags, scaleFactor) )
		);
}

} // namespace
