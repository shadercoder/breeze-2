/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beShapes.h"
#include "bePhysics/PX/beMaterial.h"
#include "bePhysics/PX/beDevice.h"
#include "bePhysics/beRigidActors.h"
#include <lean/io/raw_file.h>
#include <lean/logging/errors.h>

namespace bePhysics
{

namespace
{

// Adds all shapes of the given collection to the given shape compound.
void CreateShapesFromCollection(ShapeCompoundPX &compound, physx::PxCollection &collection, const physx::PxMaterial *pDefaultMaterial)
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
void LoadShapes(const utf8_ntri &file, ShapeCompoundPX &compound, const physx::PxMaterial *pMaterial, DevicePX &device)
{
	// HACK: Serialization requires SOME scene
	physx::PxScene *pScene = nullptr;
	device->getScenes(&pScene, 1);

	if (!pScene)
		LEAN_THROW_ERROR_MSG("Shape deserialization requires at least one scene to exist!");

	lean::raw_file shapeFile(file, lean::file::read);

	physx::PxUserReferences *pReferences = nullptr;
	physx::PxCollection *pCollection = nullptr;
	char *data = nullptr;

	try
	{
		pReferences = device->createUserReferences();
		
		if (!pReferences)
			LEAN_THROW_ERROR_MSG("PxPhysics::createUserReferences()");

		// Link actor & material in
		pReferences->setUserData(const_cast<physx::PxMaterial*>(pMaterial), reinterpret_cast<void*>(RigidActorSerializationID::DefaultMaterial));

		// Deserialize mesh data
		{
			pCollection = device->createCollection();

			if (!pCollection)
				LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");

			uint4 meshSize = 0;
			shapeFile.read( reinterpret_cast<char*>(&meshSize), sizeof(meshSize) );

			// PhysX requires 128 byte alignment!
			data = static_cast<char*>( lean::default_heap::allocate<128>(meshSize) );

			shapeFile.read(data, meshSize);

			if (!pCollection->deserialize(data, pReferences, nullptr))
				LEAN_THROW_ERROR_MSG("PxCollection::deserialize()");

			device->addCollection(*pCollection, *pScene);
			
			device->releaseCollection(*pCollection);
			pCollection = nullptr;

			device.FreeOnRelease(data, 16);
			data = nullptr;
		}
		
		// Deserialize shape data
		{
			pCollection = device->createCollection();

			if (!pCollection)
				LEAN_THROW_ERROR_MSG("PxPhysics::createCollection()");

			uint4 shapesSize = 0;
			shapeFile.read( reinterpret_cast<char*>(&shapesSize), sizeof(shapesSize) );

			// PhysX requires 128 byte alignment!
			data = static_cast<char*>( lean::default_heap::allocate<128>(shapesSize) );

			shapeFile.read(data, shapesSize);

			if (!pCollection->deserialize(data, nullptr, pReferences))
				LEAN_THROW_ERROR_MSG("PxCollection::deserialize()");

			CreateShapesFromCollection(compound, *pCollection, pMaterial);

			device->releaseCollection(*pCollection);
			pCollection = nullptr;

			lean::default_heap::free<16>(data);
			data = nullptr;
		}
		
		device->releaseUserReferences(*pReferences);
		pReferences = nullptr;
	}
	catch (...)
	{
		if (pCollection)
			device->releaseCollection(*pCollection);

		lean::default_heap::free<16>(data);

		if (pReferences)
			device->releaseUserReferences(*pReferences);

		throw;
	}
}

} // namespace

// Creates a shape from the given shape file.
lean::resource_ptr<ShapeCompound, true> LoadShape(const utf8_ntri &file, const Material *pMaterial, Device &device)
{
	lean::resource_ptr<ShapeCompoundPX> pCompound = lean::bind_resource( new ShapeCompoundPX() );

	LoadShapes(file, *pCompound, ToImpl(*pMaterial), ToImpl(device));

	return pCompound.transfer();
}

} // namespace