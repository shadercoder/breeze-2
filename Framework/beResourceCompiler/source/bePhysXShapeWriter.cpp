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
#include <lean/memory/chunk_heap.h>
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
		// WARNING: PhysX passes nullptrs from time to time
		if (buffer)
		{
			memcpy(buffer, Memory, size);
			Memory += size;
		}
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

struct FileStream : public physx::PxOutputStream
{
	lean::raw_file *File;
	uint8 Base;

	FileStream(lean::raw_file *pFile)
		: File(pFile),
		Base(pFile->pos()) { }

	physx::PxU32 write(const void* buffer, physx::PxU32 size)
	{
		// WARNING: PhysX passes nullptrs from time to time
		if (buffer)
			return File->write( static_cast<const char*>(buffer), size );
//		else
//			LEAN_LOG_ERROR("PhysX nullptr buffer caught while writing to file");

		return size;
	}

	physx::PxU32 getTotalStoredSize()
	{
		return static_cast<uint4>( File->pos() - Base );
	}
};

struct MeshRef
{

};

struct SerializationContext
{
	physx::PxPhysics &physics;
	physx::PxCooking &cooking;
	physx::PxCollection &meshCollection;
	physx::PxCollection &shapeCollection;
	physx::PxMaterial &defaultMaterial;
	physx::PxRigidActor &actor;
	lean::chunk_heap<1024> nameHeap;
	std::vector<physx::PxSerialObjectAndRef> meshRefs;
};

/// Adds the given name to the internal string heap.
const char *AddName(SerializationContext &context, const aiString &name)
{
	return static_cast<char*>(
			memcpy( context.nameHeap.allocate(name.length + 1), name.data, name.length + 1 )
		);
}

/// Saves a box according to the given transformation.
void SaveBox(SerializationContext &context, const aiMatrix4x4 &transform, const aiString &name)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;

	transform.Decompose(scale, rot, pos);
	rot.Normalize();

	LEAN_LOG("Box at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << scale.x << "; " << scale.y << "; " << scale.z << ")");

	physx::PxShape *pShape = context.actor.createShape(
		physx::PxBoxGeometry(abs(scale.x), abs(scale.y), abs(scale.z)),
		context.defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );
	pShape->setName( AddName(context, name) );

	pShape->collectForExport(context.shapeCollection);
}

/// Saves a sphere according to the given transformation.
void SaveSphere(SerializationContext &context, const aiMatrix4x4 &transform, const aiString &name)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;

	transform.Decompose(scale, rot, pos);
	rot.Normalize();

	float radius = pow( abs(scale.x * scale.y * scale.z), 1.0f / 3.0f );

	LEAN_LOG("Sphere at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << radius << ")");

	physx::PxShape *pShape = context.actor.createShape(
		physx::PxSphereGeometry(radius),
		context.defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );
	pShape->setName( AddName(context, name) );

	pShape->collectForExport(context.shapeCollection);
}

/// Saves a convex mesh according to the given transformation.
void SaveConvex(SerializationContext &context, physx::PxSerialObjectAndRef &meshRef, const aiMesh &mesh, const aiMatrix4x4 &transform, const aiString &name)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;

	transform.Decompose(scale, rot, pos);
	rot.Normalize();

	LEAN_LOG("Convex at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << scale.x << "; " << scale.y << "; " << scale.z << ")");

	if (!meshRef.ref)
	{
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
		if (!context.cooking.cookConvexMesh(convexDesc, meshData))
			LEAN_THROW_ERROR_MSG("PxCooking::cookConvexMesh()");

		LEAN_LOG("Cooked convex mesh for shape " << name.C_Str());

		bePhysics::PX3::scoped_pxptr_t<physx::PxConvexMesh>::t pMesh(
				context.physics.createConvexMesh( MemoryStream(&meshData.Data[0]) )
			);
		if (!pMesh)
			LEAN_THROW_ERROR_MSG("PxPhysics::createConvexMesh()");

		LEAN_LOG("Created convex mesh for shape " << name.C_Str());

		pMesh->collectForExport(context.meshCollection);
		meshRef.serializable = pMesh.detach();
		meshRef.ref = bePhysics::RigidActorSerializationID::InternalBase + context.meshCollection.getNbObjects();
		context.meshCollection.setObjectRef(*meshRef.serializable, meshRef.ref);
	}

	physx::PxShape *pShape = context.actor.createShape(
		physx::PxConvexMeshGeometry( static_cast<physx::PxConvexMesh*>(meshRef.serializable) ),
		context.defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );
	pShape->setName( AddName(context, name) );

	context.shapeCollection.addExternalRef(*meshRef.serializable, meshRef.ref);
	pShape->collectForExport(context.shapeCollection);
}

/// Saves a triangle mesh according to the given transformation.
void SaveMesh(SerializationContext &context, physx::PxSerialObjectAndRef &meshRef, const aiMesh &mesh, const aiMatrix4x4 &transform, const aiString &name)
{
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;

	transform.Decompose(scale, rot, pos);
	rot.Normalize();

	LEAN_LOG("Triangle Mesh at " << pos.x << "; " << pos.y << "; " << pos.z
		<< " (" << scale.x << "; " << scale.y << "; " << scale.z << ")");

	if (!meshRef.ref)
	{
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
		if (!context.cooking.cookTriangleMesh(meshDesc, meshData))
			LEAN_THROW_ERROR_MSG("PxCooking::cookTriangleMesh()");

		LEAN_LOG("Cooked mesh for shape " << name.C_Str());

		bePhysics::PX3::scoped_pxptr_t<physx::PxTriangleMesh>::t pMesh(
				context.physics.createTriangleMesh( MemoryStream(&meshData.Data[0]) )
			);
		if (!pMesh)
			LEAN_THROW_ERROR_MSG("PxPhysics::createTriangleMesh()");

		LEAN_LOG("Created mesh for shape " << name.C_Str());

		pMesh->collectForExport(context.meshCollection);
		meshRef.serializable = pMesh.detach();
		meshRef.ref = bePhysics::RigidActorSerializationID::InternalBase + context.meshCollection.getNbObjects();
		context.meshCollection.setObjectRef(*meshRef.serializable, meshRef.ref);
	}

	physx::PxShape *pShape = context.actor.createShape(
		physx::PxTriangleMeshGeometry( static_cast<physx::PxTriangleMesh*>(meshRef.serializable) ),
		context.defaultMaterial,
		physx::PxTransform( physx::PxVec3(pos.x, pos.y, pos.z), physx::PxQuat(rot.x, rot.y, rot.z, rot.w) ) );
	pShape->setName( AddName(context, name) );

	context.shapeCollection.addExternalRef(*meshRef.serializable, meshRef.ref);
	pShape->collectForExport(context.shapeCollection);
}

/// Saves the given shape mesh.
void SaveShape(SerializationContext &context, physx::PxSerialObjectAndRef &meshRef, const aiMesh &mesh, const aiString &nodeName, const aiMatrix4x4 &transform)
{
	bool bUnspecified = false;

	const aiString &meshName = (mesh.mName.length != 0) ? mesh.mName : nodeName;
	const aiString &shapeName = (nodeName.length != 0) ? nodeName : mesh.mName;

	if (meshName.length != 0)
	{
		LEAN_LOG("Cooking mesh " << meshName.data);

		if (_strnicmp(meshName.data, "box", lean::ntarraylen("box")) == 0 || _strnicmp(meshName.data, "cube", lean::ntarraylen("cube")) == 0)
			SaveBox(context, transform, shapeName);
		else if (_strnicmp(meshName.data, "sphere", lean::ntarraylen("sphere")) == 0 || _strnicmp(meshName.data, "ball", lean::ntarraylen("ball")) == 0)
			SaveSphere(context, transform, shapeName);
		else if (_strnicmp(meshName.data, "convex", lean::ntarraylen("convex")) == 0)
			SaveConvex(context, meshRef, mesh, transform, shapeName);
		else
			bUnspecified = true;
	}
	else
	{
		LEAN_LOG("Cooking unnamed mesh");

		bUnspecified = true;
	}

	if (bUnspecified)
		SaveMesh(context, meshRef, mesh, transform, shapeName);
}

/// Saves all shapes in the given node.
void SaveShapes(SerializationContext &context, const aiNode &node, const aiMatrix4x4 &parentTransform, const aiScene &scene)
{
	aiMatrix4x4 transform = parentTransform * node.mTransformation;

	for (uint4 i = 0; i < node.mNumMeshes; ++i)
		SaveShape(context, context.meshRefs[node.mMeshes[i]], *scene.mMeshes[node.mMeshes[i]], node.mName, transform);

	for (uint4 i = 0; i < node.mNumChildren; ++i)
		SaveShapes(context, *node.mChildren[i], transform, scene);
}

} // namespace

// Saves a physical representation of the given mesh to the given file.
void SavePhysXShapes(const utf8_ntri &file, const Scene &scene, PhysicsCooker &cooker)
{
	physx::PxPhysics *physics = cooker.GetData().Physics.get();
	physx::PxCooking *cooking = cooker.GetData().Cooking.get();

	const SceneImpl &sceneImpl = static_cast<const SceneImpl&>(scene);

	lean::raw_file shapeFile(file, lean::file::readwrite, lean::file::overwrite);


	// Default material
	bePhysics::PX3::scoped_pxptr_t<physx::PxMaterial>::t defaultMaterial( physics->createMaterial(0.5f, 0.5f, 0.5f) );
	if (!defaultMaterial)
		LEAN_THROW_ERROR_MSG("PxPhysics::createMaterial()");

	// Dummy actor
	bePhysics::PX3::scoped_pxptr_t<physx::PxRigidActor>::t actor(
			physics->createRigidDynamic( physx::PxTransform::createIdentity() )
		);
	if (!actor)
		LEAN_THROW_ERROR_MSG("PxPhysics::createRigidDynamic()");

	// Target collection
	bePhysics::PX3::scoped_pxptr_t<physx::PxCollection>::t collection( physics->createCollection() );
	if (!collection)
		LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");


	SerializationContext context = { *physics, *cooking, *collection, *collection, *defaultMaterial, *actor };
	{
		physx::PxSerialObjectAndRef noRef = { };
		context.meshRefs.resize(sceneImpl.GetScene()->mNumMeshes, noRef);
	}

	// Add shapes to collection
	SaveShapes(context, *sceneImpl.GetScene()->mRootNode, aiMatrix4x4(), *sceneImpl.GetScene());
	LEAN_LOG("Shapes collected");

	// Provide links for actor & material
	actor->collectForExport(*collection);
	collection->setUserData(*actor, bePhysics::RigidActorSerializationID::Actor);
//	collection->addExternalRef(pActor, reinterpret_cast<void*>(bePhysics::RigidActorSerializationID::Actor));
	collection->addExternalRef(*defaultMaterial, bePhysics::RigidActorSerializationID::DefaultMaterial);
	LEAN_LOG("Prototype actor collected");

	// Serialize shapes
	{
		LEAN_LOG("Starting serialization");

		uint4 shapeSize = 0;
		shapeFile.write( reinterpret_cast<const char*>(&shapeSize), sizeof(shapeSize) );
		
		LEAN_LOG("Shape collection");
		FileStream shapeStream(&shapeFile);
		collection->serialize(shapeStream, true);
		
		shapeSize = shapeStream.getTotalStoredSize();

		LEAN_LOG("Chunk");
		uint8 pos = shapeFile.pos();
		shapeFile.pos( shapeStream.Base - sizeof(shapeSize) );
		shapeFile.write( reinterpret_cast<const char*>(&shapeSize), sizeof(shapeSize) );
		shapeFile.pos(pos);

		LEAN_LOG("Serialization complete");
	}
}

} // namespace
