/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beShapes.h"
#include "bePhysics/PX3/beShapeCache.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beDevice.h"
#include "bePhysics/beRigidActors.h"

#include <lean/io/raw_file.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace bePhysics
{

namespace PX3
{

namespace
{

// Adds all shapes of the given collection to the given shape compound.
void CreateShapesFromCollection(ShapeCompound &compound, physx::PxCollection &collection, const physx::PxMaterial *pDefaultMaterial)
{
	const uint4 objectCount = collection.getNbObjects();

	for (uint4 i = 0; i < objectCount; ++i)
	{
		physx::PxSerializable *pObject = collection.getObject(i);

		if (pObject->getConcreteType() == physx::PxConcreteType::eSHAPE)
		{
			physx::PxShape *pShape = static_cast<physx::PxShape*>(pObject);

			physx::PxMaterial *pMaterial = const_cast<physx::PxMaterial*>(pDefaultMaterial);
			pShape->getMaterials(&pMaterial, 1);

			switch (pShape->getGeometryType())
			{
			case physx::PxGeometryType::eBOX:
				{
					physx::PxBoxGeometry geom;
					pShape->getBoxGeometry(geom);
					compound.AddShape(geom, pMaterial, pShape->getLocalPose());
				}
				break;
			case physx::PxGeometryType::eSPHERE:
				{
					physx::PxSphereGeometry geom;
					pShape->getSphereGeometry(geom);
					compound.AddShape(geom, pMaterial, pShape->getLocalPose());
				}
				break;
			case physx::PxGeometryType::eCONVEXMESH:
				{
					physx::PxConvexMeshGeometry geom;
					pShape->getConvexMeshGeometry(geom);
					compound.AddShape(geom, pMaterial, pShape->getLocalPose());
				}
				break;
			case physx::PxGeometryType::eTRIANGLEMESH:
				{
					physx::PxTriangleMeshGeometry geom;
					pShape->getTriangleMeshGeometry(geom);
					compound.AddShape(geom, pMaterial, pShape->getLocalPose());
				}
				break;
			}
		}
	}
}

/// Reads a collection of shapes from the given file.
void LoadShapes(const utf8_ntri &file, ShapeCompound &compound, const physx::PxMaterial *pMaterial, Device &device)
{
	// HACK: Serialization requires SOME scene
	physx::PxScene *pScene = nullptr;
	device->getScenes(&pScene, 1);

	if (!pScene)
		LEAN_THROW_ERROR_MSG("Shape deserialization requires at least one scene to exist!");

	lean::raw_file shapeFile(file, lean::file::read);

	scoped_pxptr_t<physx::PxUserReferences>::t references( device->createUserReferences() );

	if (!references)
		LEAN_THROW_ERROR_MSG("PxPhysics::createUserReferences()");

	// Link to default material
	if (pMaterial)
		references->setUserData(const_cast<physx::PxMaterial*>(pMaterial), RigidActorSerializationID::DefaultMaterial);
	// NOTE: Default actor currently not supported by PhysX

	char *data = nullptr;

	// Deserialize mesh data
	try
	{
		scoped_pxptr_t<physx::PxCollection>::t collection( device->createCollection() );

		if (!collection)
			LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");

		uint4 meshSize = 0;
		shapeFile.read( reinterpret_cast<char*>(&meshSize), sizeof(meshSize) );

		// PhysX requires 128 byte alignment!
		data = static_cast<char*>( lean::default_heap::allocate<PX_SERIAL_FILE_ALIGN>(meshSize) );

		shapeFile.read(data, meshSize);
		
		if (!collection->deserialize(data, references, nullptr))
			LEAN_THROW_ERROR_MSG("PxCollection::deserialize()");

		device->addCollection(*collection, *pScene);

		// NOTE: De-serialization works in-place, memory has to be kept until release
		// TODO: More control on life time?
		device.FreeOnRelease(data, PX_SERIAL_FILE_ALIGN);
		data = nullptr;
	}
	catch (...)
	{
		lean::default_heap::free<PX_SERIAL_FILE_ALIGN>(data);
		throw;
	}

	data = nullptr;

	// Deserialize shape data
	try
	{
		scoped_pxptr_t<physx::PxCollection>::t collection( device->createCollection() );

		if (!collection)
			LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");

		uint4 shapesSize = 0;
		shapeFile.read( reinterpret_cast<char*>(&shapesSize), sizeof(shapesSize) );

		// PhysX requires 128 byte alignment!
		data = static_cast<char*>( lean::default_heap::allocate<PX_SERIAL_FILE_ALIGN>(shapesSize) );
		
		shapeFile.read(data, shapesSize);

		if (!collection->deserialize(data, nullptr, references))
			LEAN_THROW_ERROR_MSG("PxCollection::deserialize()");

		CreateShapesFromCollection(compound, *collection, pMaterial);

		// MONITOR: WORKAROUND: Apparently, PhysX 3.2 does not fully release collections until final destruction of SDK interface object
		// TODO: More control on life time?
		device.FreeOnRelease(data, PX_SERIAL_FILE_ALIGN);
//		lean::default_heap::free<PX_SERIAL_FILE_ALIGN>(data);
		data = nullptr;
	}
	catch (...)
	{
		lean::default_heap::free<PX_SERIAL_FILE_ALIGN>(data);
		throw;
	}
}

} // namespace

} // namespace

// Creates a shape from the given shape file.
lean::resource_ptr<ShapeCompound, true> LoadShape(const utf8_ntri &file, const Material *pMaterial, Device &device, ShapeCache *pShapeCache)
{
	lean::resource_ptr<PX3::ShapeCompound> pCompound = lean::bind_resource( new PX3::ShapeCompound(pShapeCache) );

	LEAN_LOG("Attempting to load shape \"" << file.c_str() << "\"");

	PX3::LoadShapes(file, *pCompound, (pMaterial) ? ToImpl(pMaterial)->Get() : nullptr, ToImpl(device));

	LEAN_LOG("Shape \"" << file.c_str() << "\" created successfully");

	return pCompound.transfer();
}

} // namespace