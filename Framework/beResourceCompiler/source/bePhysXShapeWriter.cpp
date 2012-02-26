/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/beMeshSerialization.h"
#include "beResourceCompiler/beSceneImpl.h"
#include "beResourceCompiler/bePhysicsCookerImpl.h"
#include <bePhysics/beRigidActors.h>
#include <lean/io/raw_file.h>
#include <lean/meta/strip.h>
#include <lean/logging/log.h>
#include <lean/logging/errors.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <PxPhysicsAPI.h>
#include <PxCooking.h>
#include <common/PxIO.h>

namespace beResourceCompiler
{

namespace
{

struct MemoryStream : public physx::PxInputStream
{
	const char *Memory;

	MemoryStream(const char *memory)
		: Memory(memory) { }

	physx::PxU32 read(void* buffer, physx::PxU32 size)
	{
		memcpy(buffer, Memory, size);
		Memory += size;
		return size;
	}
};

struct VectorStream : public physx::PxOutputStream
{
	std::vector<char> Data;

	physx::PxU32 write(const void* buffer, physx::PxU32 size)
	{
		Data.insert( Data.end(), static_cast<const char*>(buffer), static_cast<const char*>(buffer) + size );
		return size;
	}
};

struct FileStream : public physx::PxSerialStream
{
	lean::raw_file *File;
	uint8 Base;

	FileStream(lean::raw_file *pFile)
		: File(pFile),
		Base(pFile->pos()) { }

	void storeBuffer(const void* buffer, physx::PxU32 size)
	{
		// WARNING: PhysX passes nullptrs from time to time
		if (buffer)
			File->write( static_cast<const char*>(buffer), size );
//		else
//			LEAN_LOG_ERROR("PhysX nullptr buffer caught while writing to file");
	}
	physx::PxU32 getTotalStoredSize()
	{
		return static_cast<uint4>( File->pos() - Base );
	}
};

/// Saves a box according to the given transformation.
void SaveBox(physx::PxCollection &shapeCollection, physx::PxMaterial &defaultMaterial, physx::PxRigidActor &actor,
	const aiMatrix4x4 &transform)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;
	transform.Decompose(scale, rot, pos);

	LEAN_LOG("Box at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << scale.x << "; " << scale.y << "; " << scale.z << ")");

	physx::PxShape *pShape = actor.createShape(
		physx::PxBoxGeometry(abs(scale.x), abs(scale.y), abs(scale.z)),
		defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );

	pShape->collectForExport(shapeCollection);
}

/// Saves a sphere according to the given transformation.
void SaveSphere(physx::PxCollection &shapeCollection, physx::PxMaterial &defaultMaterial, physx::PxRigidActor &actor,
	const aiMatrix4x4 &transform)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;
	transform.Decompose(scale, rot, pos);

	float radius = pow( abs(scale.x * scale.y * scale.z), 1.0f / 3.0f );

	LEAN_LOG("Sphere at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << radius << ")");

	physx::PxShape *pShape = actor.createShape(
		physx::PxSphereGeometry(radius),
		defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );

	pShape->collectForExport(shapeCollection);
}

/// Saves a convex mesh according to the given transformation.
void SaveConvex(physx::PxCollection &meshCollection, physx::PxCollection &shapeCollection, physx::PxMaterial &defaultMaterial, physx::PxRigidActor &actor,
	physx::PxCooking &cooking, physx::PxPhysics &physics,
	const aiMesh &mesh, const aiMatrix4x4 &transform)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;
	transform.Decompose(scale, rot, pos);

	LEAN_LOG("Convex at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << scale.x << "; " << scale.y << "; " << scale.z << ")");

	std::vector<aiVector3D> vertices(mesh.mNumVertices);

	for (size_t i = 0; i < mesh.mNumVertices; ++i)
		vertices[i] = mesh.mVertices[i].SymMul(scale);

	std::vector<uint4> indices(3 * mesh.mNumFaces);

	for (size_t i = 0; i < mesh.mNumFaces; ++i)
		memcpy(&indices[3 * i], mesh.mFaces[i].mIndices, sizeof(uint4) * 3);

	physx::PxConvexMeshDesc convexDesc;
	convexDesc.points.data = &vertices[0];
	convexDesc.points.stride = sizeof(aiVector3D);
	convexDesc.points.count = mesh.mNumVertices;
	convexDesc.triangles.data = &indices[0];
	convexDesc.triangles.count = mesh.mNumFaces;
	convexDesc.triangles.stride = 3 * sizeof(uint4);
	convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	VectorStream meshData;
	if (!cooking.cookConvexMesh(convexDesc, meshData))
		LEAN_THROW_ERROR_MSG("PxCooking::cookConvexMesh()");

	bePhysics::scoped_pxptr_t<physx::PxConvexMesh>::t pMesh(
			physics.createConvexMesh( MemoryStream(&meshData.Data[0]) )
		);

	if (!pMesh)
		LEAN_THROW_ERROR_MSG("PxPhysics::createConvexMesh()");

	void *meshSerializationID = reinterpret_cast<void*>(bePhysics::RigidActorSerializationID::InternalBase + meshCollection.getNbObjects());
	pMesh->collectForExport(meshCollection);
	meshCollection.setUserData(*pMesh, meshSerializationID);

	physx::PxShape *pShape = actor.createShape(
		physx::PxConvexMeshGeometry( pMesh ),
		defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );

	shapeCollection.addExternalRef(*pMesh.detach(), meshSerializationID);
	pShape->collectForExport(shapeCollection);
}

/// Saves a triangle mesh according to the given transformation.
void SaveMesh(physx::PxCollection &meshCollection, physx::PxCollection &shapeCollection, physx::PxMaterial &defaultMaterial, physx::PxRigidActor &actor,
	physx::PxCooking &cooking, physx::PxPhysics &physics,
	const aiMesh &mesh, const aiMatrix4x4 &transform)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;
	transform.Decompose(scale, rot, pos);

	LEAN_LOG("Triangle Mesh at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << scale.x << "; " << scale.y << "; " << scale.z << ")");

	std::vector<aiVector3D> vertices(mesh.mNumVertices);

	for (size_t i = 0; i < mesh.mNumVertices; ++i)
		vertices[i] = mesh.mVertices[i].SymMul(scale);

	std::vector<uint4> indices(3 * mesh.mNumFaces);

	for (size_t i = 0; i < mesh.mNumFaces; ++i)
		memcpy(&indices[3 * i], mesh.mFaces[i].mIndices, sizeof(uint4) * 3);

	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.data = &vertices[0];
	meshDesc.points.stride = sizeof(aiVector3D);
	meshDesc.points.count = mesh.mNumVertices;
	meshDesc.triangles.data = &indices[0];
	meshDesc.triangles.count = mesh.mNumFaces;
	meshDesc.triangles.stride = 3 * sizeof(uint4);
	meshDesc.isValid();

	VectorStream meshData;
	if (!cooking.cookTriangleMesh(meshDesc, meshData))
		LEAN_THROW_ERROR_MSG("PxCooking::cookTriangleMesh()");

	bePhysics::scoped_pxptr_t<physx::PxTriangleMesh>::t pMesh(
			physics.createTriangleMesh( MemoryStream(&meshData.Data[0]) )
		);

	if (!pMesh)
		LEAN_THROW_ERROR_MSG("PxPhysics::createTriangleMesh()");

	void *meshSerializationID = reinterpret_cast<void*>(bePhysics::RigidActorSerializationID::InternalBase + meshCollection.getNbObjects());
	pMesh->collectForExport(meshCollection);
	meshCollection.setUserData(*pMesh, meshSerializationID);

	physx::PxShape *pShape = actor.createShape(
		physx::PxTriangleMeshGeometry( pMesh ),
		defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );

	shapeCollection.addExternalRef(*pMesh.detach(), meshSerializationID);
	pShape->collectForExport(shapeCollection);
}

/// Saves the given shape mesh.
void SaveShape(physx::PxCollection &meshCollection, physx::PxCollection &shapeCollection, physx::PxMaterial &defaultMaterial, physx::PxRigidActor &actor,
	physx::PxCooking &cooking, physx::PxPhysics &physics,
	const aiMesh &mesh, const aiString &nodeName, const aiMatrix4x4 &transform)
{
	bool bUnspecified = false;

	const aiString &meshName = (mesh.mName.length != 0) ? mesh.mName : nodeName;

	if (meshName.length != 0)
	{
		LEAN_LOG("Cooking mesh " << meshName.data);

		if (_strnicmp(meshName.data, "box", lean::ntarraylen("box")) == 0 || _strnicmp(meshName.data, "cube", lean::ntarraylen("cube")) == 0)
			SaveBox(shapeCollection, defaultMaterial, actor, transform);
		else if (_strnicmp(meshName.data, "sphere", lean::ntarraylen("sphere")) == 0 || _strnicmp(meshName.data, "ball", lean::ntarraylen("ball")) == 0)
			SaveSphere(shapeCollection, defaultMaterial, actor, transform);
		else if (_strnicmp(meshName.data, "convex", lean::ntarraylen("convex")) == 0)
			SaveConvex(meshCollection, shapeCollection, defaultMaterial, actor, cooking, physics, mesh, transform);
		else
			bUnspecified = true;
	}
	else
	{
		LEAN_LOG("Cooking unnamed mesh");

		bUnspecified = true;
	}

	if (bUnspecified)
		SaveMesh(meshCollection, shapeCollection, defaultMaterial, actor, cooking, physics, mesh, transform);
}

/// Saves all shapes in the given node.
void SaveShapes(physx::PxCollection &meshCollection, physx::PxCollection &shapeCollection, physx::PxMaterial &defaultMaterial, physx::PxRigidActor &actor,
	physx::PxCooking &cooking, physx::PxPhysics &physics,
	const aiNode &node, const aiMatrix4x4 &parentTransform, const aiScene &scene)
{
	aiMatrix4x4 transform = parentTransform * node.mTransformation;

	for (uint4 i = 0; i < node.mNumMeshes; ++i)
		SaveShape(meshCollection, shapeCollection, defaultMaterial, actor, cooking, physics,  *scene.mMeshes[node.mMeshes[i]], node.mName, transform);

	for (uint4 i = 0; i < node.mNumChildren; ++i)
		SaveShapes(meshCollection, shapeCollection, defaultMaterial, actor, cooking, physics, *node.mChildren[i], transform, scene);
}

} // namespace

// Saves a physical representation of the given mesh to the given file.
void SavePhysXShapes(const utf8_ntri &file, const Scene &scene, PhysicsCooker &cooker)
{
	physx::PxPhysics *pPhysics = cooker.GetData().Physics;
	physx::PxCooking *pCooking = cooker.GetData().Cooking;

	const SceneImpl &sceneImpl = static_cast<const SceneImpl&>(scene);

	lean::raw_file shapeFile(file, lean::file::readwrite, lean::file::overwrite);


	// Default material
	bePhysics::scoped_pxptr_t<physx::PxMaterial>::t pDefaultMaterial( pPhysics->createMaterial(0.5f, 0.5f, 0.5f) );

	if (!pDefaultMaterial)
		LEAN_THROW_ERROR_MSG("PxPhysics::createMaterial()");


	// Dummy actor
	bePhysics::scoped_pxptr_t<physx::PxRigidActor>::t pActor(
			pPhysics->createRigidDynamic( physx::PxTransform::createIdentity() )
		);

	if (!pActor)
		LEAN_THROW_ERROR_MSG("PxPhysics::createRigidDynamic()");


	// Target collections
	physx::PxCollection *pMeshCollection = pPhysics->createCollection();
	physx::PxCollection *pShapeCollection = pPhysics->createCollection();
	
	if (!pMeshCollection || !pShapeCollection)
		LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");


	// Add shapes to collection
	SaveShapes(*pMeshCollection, *pShapeCollection, *pDefaultMaterial, *pActor, *pCooking, *pPhysics,
		*sceneImpl.GetScene()->mRootNode, aiMatrix4x4(), *sceneImpl.GetScene());

	// Provide links for actor & material
	pShapeCollection->addExternalRef(*pDefaultMaterial, reinterpret_cast<void*>(bePhysics::RigidActorSerializationID::DefaultMaterial));
	pActor->collectForExport(*pShapeCollection);
	pShapeCollection->setUserData(*pActor, reinterpret_cast<void*>(bePhysics::RigidActorSerializationID::Actor));
//	pCollection->addExternalRef(pActor, reinterpret_cast<void*>(bePhysics::RigidActorSerializationID::Actor));

	// Serialize meshes
	{
		uint4 meshSize = 0;
		shapeFile.write( reinterpret_cast<const char*>(&meshSize), sizeof(meshSize) );
	
		FileStream meshStream(&shapeFile);
		pMeshCollection->serialize(meshStream);
	
		meshSize = meshStream.getTotalStoredSize();

		uint8 pos = shapeFile.pos();
		shapeFile.pos( meshStream.Base - sizeof(meshSize) );
		shapeFile.write( reinterpret_cast<const char*>(&meshSize), sizeof(meshSize) );
		shapeFile.pos(pos);
	}

	// Serialize shapes
	{
		uint4 shapeSize = 0;
		shapeFile.write( reinterpret_cast<const char*>(&shapeSize), sizeof(shapeSize) );
		
		FileStream shapeStream(&shapeFile);
		pShapeCollection->serialize(shapeStream);
		
		shapeSize = shapeStream.getTotalStoredSize();

		uint8 pos = shapeFile.pos();
		shapeFile.pos( shapeStream.Base - sizeof(shapeSize) );
		shapeFile.write( reinterpret_cast<const char*>(&shapeSize), sizeof(shapeSize) );
		shapeFile.pos(pos);
	}

	// MONITOR: Manual release required - not exception safe!
	pPhysics->releaseCollection(*pMeshCollection);
	pPhysics->releaseCollection(*pShapeCollection);
}

} // namespace
