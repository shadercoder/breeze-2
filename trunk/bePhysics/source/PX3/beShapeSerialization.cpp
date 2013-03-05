/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beAssembledShape.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/beRigidActors.h"

#include <lean/io/mapped_file.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace bePhysics
{

namespace PX3
{

namespace
{

/// Reads a value of the given type, incrementing the read pointer.
template <class Data>
LEAN_INLINE const Data& ReadData(const char *&ptr)
{
	const Data &data = *reinterpret_cast<const Data*>(ptr);
	ptr += sizeof(Data);
	return data;
}

} // namespace

/// Reads a collection of shapes from the given file.
lean::resource_ptr<AssembledShape, true> LoadShape(const char *srcData, uint8 srcDataLength, const physx::PxMaterial *pMaterial, PX3::Device &device)
{
	lean::resource_ptr<PX3::AssembledShape> assembledShape;

	// Inject external dependencies
	scoped_pxptr_t<physx::PxUserReferences>::t references( device->createUserReferences() );
	if (!references)
		LEAN_THROW_ERROR_MSG("PxPhysics::createUserReferences()");

	// Link to default material
	if (pMaterial)
		references->setUserData(const_cast<physx::PxMaterial*>(pMaterial), RigidActorSerializationID::DefaultMaterial);
	
	// NOTE: Default actor currently not supported by PhysX

	const char *srcDataEnd = srcData + srcDataLength;
	uint4 shapeSize = ReadData<uint4>(srcData);

	// PhysX requires 128 byte alignment!
	char *data = static_cast<char*>( PhysXSerializationAllocate(shapeSize) );
	bool bCollectionLost = false;

	// Deserialize shape data
	try
	{
		memcpy(data, srcData, srcDataEnd - srcData);

		scoped_pxptr_t<physx::PxCollection>::t collection( device->createCollection() );
		if (!collection)
			LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");
		if (!collection->deserialize(data, references.get(), references.get()))
			LEAN_THROW_ERROR_MSG("PxCollection::deserialize()");

		// NOTE: Cannot free memory until end of program once collection of unidentified objects has been loaded
		bCollectionLost = true;

		// Extract prototype actor
		physx::PxRigidDynamic *actor = references->getObjectFromRef(RigidActorSerializationID::Actor)->is<physx::PxRigidDynamic>();
		if (!actor)
			LEAN_THROW_ERROR_MSG("Shape deserialization requires prototype actor");

		// IMPORTANT: Need to release application resource references manually!
		for (uint4 i = 0, count = actor->getNbShapes(); i < count; ++i)
		{
			physx::PxShape *shape = nullptr;
			actor->getShapes(&shape, 1, i);
			switch (shape->getGeometryType())
			{
			case px::PxGeometryType::eTRIANGLEMESH:
				{
					px::PxTriangleMeshGeometry geom;
					shape->getTriangleMeshGeometry(geom);
					geom.triangleMesh->release();
				}
				break;
			case px::PxGeometryType::eCONVEXMESH:
				{
					px::PxConvexMeshGeometry geom;
					shape->getConvexMeshGeometry(geom);
					geom.convexMesh->release();
				}
				break;
			}
		}

		// Transfer ownership of actor & data to assembled shape
		assembledShape = new_resource AssembledShape(actor, data);
		bCollectionLost = false;
	}
	catch (...)
	{
		if (bCollectionLost)
			device.SerializationFreeOnRelease(data);
		else
			PhysXSerializationFree(data);
		throw;
	}

	return assembledShape.transfer();
}

} // namespace

// Creates a shape from the given shape data block.
lean::resource_ptr<AssembledShape, lean::critical_ref> LoadShape(const void *data, uint8 dataLength, const Material *pMaterial, Device &device)
{
	return PX3::LoadShape(
			reinterpret_cast<const char*>(data), dataLength, 
			(pMaterial) ? ToImpl(pMaterial)->Get() : nullptr,
			ToImpl(device)
		);
}

// Creates a shape from the given shape file.
lean::resource_ptr<AssembledShape, true> LoadShape(const utf8_ntri &file, const Material *pMaterial, Device &device)
{
	LEAN_LOG("Attempting to load shape \"" << file.c_str() << "\"");

	lean::rmapped_file shapeFile(file);
	lean::resource_ptr<AssembledShape> compound = LoadShape(shapeFile.data(), shapeFile.size(), pMaterial, device);
	
	LEAN_LOG("Shape \"" << file.c_str() << "\" created successfully");
	
	return compound.transfer();
}

} // namespace