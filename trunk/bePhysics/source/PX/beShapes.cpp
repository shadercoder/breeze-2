/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX/beShapes.h"
#include "bePhysics/PX/beDevice.h"
#include <lean/logging/errors.h>

namespace bePhysics
{

// Constructor.
ShapeCompoundPX::ShapeCompoundPX()
{
}

// Destructor.
ShapeCompoundPX::~ShapeCompoundPX()
{
}

// Adds the given box.
void ShapeCompoundPX::AddShape(const physx::PxBoxGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose)
{
	m_boxes.push_back( box_shape(shape, pMaterial, pose) );
	RebuildGeometry();
}

// Adds the given sphere.
void ShapeCompoundPX::AddShape(const physx::PxSphereGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose)
{
	m_spheres.push_back( sphere_shape(shape, pMaterial, pose) );
	RebuildGeometry();
}

// Adds the given convex mesh.
void ShapeCompoundPX::AddShape(const physx::PxConvexMeshGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose)
{
	m_convexes.push_back( convex_shape(shape, pMaterial, pose) );
	RebuildGeometry();
}

// Adds the given triangle mesh.
void ShapeCompoundPX::AddShape(const physx::PxTriangleMeshGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose)
{
	m_meshes.push_back( mesh_shape(shape, pMaterial, pose) );
	RebuildGeometry();
}

namespace
{

/// Appens pointers to the elements in the given range.
template <class Geometry, class Materials, class Poses, class Iterator>
void AddShapePointers(Geometry &geometry, Materials &materials, Poses &poses, Iterator begin, Iterator end)
{
	for (Iterator it = begin; it < end; ++it)
	{
		geometry.push_back( &it->geom );
		materials.push_back( it->mat );
		poses.push_back( it->pose );
	}
}

}

// Rebuilds the geometry vector.
void ShapeCompoundPX::RebuildGeometry()
{
	m_geometry.clear();
	m_materials.clear();
	m_poses.clear();

	AddShapePointers(m_geometry, m_materials, m_poses, m_boxes.begin(), m_boxes.end());
	AddShapePointers(m_geometry, m_materials, m_poses, m_spheres.begin(), m_spheres.end());
	AddShapePointers(m_geometry, m_materials, m_poses, m_convexes.begin(), m_convexes.end());
	AddShapePointers(m_geometry, m_materials, m_poses, m_meshes.begin(), m_meshes.end());
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxShape &shape, const physx::PxVec3 &scaling)
{
	physx::PxTransform pose = shape.getLocalPose();

	switch (shape.getGeometryType())
	{
	case physx::PxGeometryType::eBOX:
		{
			physx::PxBoxGeometry geom;
			shape.getBoxGeometry(geom);
			Scale(geom, pose, scaling);
			shape.setGeometry(geom);
		}
		break;
	case physx::PxGeometryType::eSPHERE:
		{
			physx::PxSphereGeometry geom;
			shape.getSphereGeometry(geom);
			Scale(geom, pose, scaling);
			shape.setGeometry(geom);
		}
		break;
	case physx::PxGeometryType::ePLANE:
		{
			physx::PxPlaneGeometry geom;
			shape.getPlaneGeometry(geom);
			Scale(geom, pose, scaling);
			shape.setGeometry(geom);
		}
		break;
	case physx::PxGeometryType::eCAPSULE:
		{
			physx::PxCapsuleGeometry geom;
			shape.getCapsuleGeometry(geom);
			Scale(geom, pose, scaling);
			shape.setGeometry(geom);
		}
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		{
			physx::PxConvexMeshGeometry geom;
			shape.getConvexMeshGeometry(geom);
			Scale(geom, pose, scaling);
			shape.setGeometry(geom);
		}
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		{
			physx::PxTriangleMeshGeometry geom;
			shape.getTriangleMeshGeometry(geom);
			Scale(geom, pose, scaling);
			shape.setGeometry(geom);
		}
		break;
	}

	shape.setLocalPose(pose);
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	switch (geometry.getType())
	{
	case physx::PxGeometryType::eBOX:
		Scale( static_cast<physx::PxBoxGeometry&>(geometry), pose, scaling );
		break;
	case physx::PxGeometryType::eSPHERE:
		Scale( static_cast<physx::PxSphereGeometry&>(geometry), pose, scaling );
		break;
	case physx::PxGeometryType::ePLANE:
		Scale( static_cast<physx::PxPlaneGeometry&>(geometry), pose, scaling );
		break;
	case physx::PxGeometryType::eCAPSULE:
		Scale( static_cast<physx::PxCapsuleGeometry&>(geometry), pose, scaling );
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		Scale( static_cast<physx::PxConvexMeshGeometry&>(geometry), pose, scaling );
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		Scale( static_cast<physx::PxTriangleMeshGeometry&>(geometry), pose, scaling );
		break;
	}
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxBoxGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	// Shape-space approximation
	physx::PxMat33 scaleMat = physx::PxMat33::createDiagonal(scaling) * physx::PxMat33(pose.q); // == (pose^-1 * scaling)^T
	geometry.halfExtents.x *= scaleMat.column0.magnitude();
	geometry.halfExtents.y *= scaleMat.column1.magnitude();
	geometry.halfExtents.z *= scaleMat.column2.magnitude();

	pose.p = pose.p.multiply(scaling);
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxSphereGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	// Omnidirectional approximation
	geometry.radius *= pow( abs(scaling.x * scaling.y * scaling.z), 1.0f / 3.0f );

	pose.p = pose.p.multiply(scaling);
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxPlaneGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	pose.p = pose.p.multiply(scaling);
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxCapsuleGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	// Shape-space approximation
	physx::PxMat33 scaleMat = physx::PxMat33::createDiagonal(scaling) * physx::PxMat33(pose.q); // == (pose^-1 * scaling)^T
	geometry.halfHeight *= scaleMat.column0.magnitude();
	// Bi-directional shape-space approximation
	geometry.radius *= sqrt(scaleMat.column1.magnitude() * scaleMat.column2.magnitude());

	pose.p = pose.p.multiply(scaling);
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxConvexMeshGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	// TODO: Remove WORKAROUND as soon as possible

	// Shape-space approximation
	physx::PxMat33 scaleMat = physx::PxMat33::createDiagonal(scaling) * physx::PxMat33(pose.q); // == (pose^-1 * scaling)^T
	geometry.scale.scale.x *= scaleMat.column0.magnitude();
	geometry.scale.scale.y *= scaleMat.column1.magnitude();
	geometry.scale.scale.z *= scaleMat.column2.magnitude();
	
/*	// MONITOR: only allows for one kind of scaling
	geometry.scale.rotation = pose.q.getConjugate();
	geometry.scale.scale = geometry.scale.scale.multiply(scaling);
*/
	pose.p = pose.p.multiply(scaling);
}

// Scales the given shape as precisely as possible.
void Scale(physx::PxTriangleMeshGeometry &geometry, physx::PxTransform &pose, const physx::PxVec3 &scaling)
{
	// TODO: Remove WORKAROUND as soon as possible

	// Shape-space approximation
	physx::PxMat33 scaleMat = physx::PxMat33::createDiagonal(scaling) * physx::PxMat33(pose.q); // == (pose^-1 * scaling)^T
	geometry.scale.scale.x *= scaleMat.column0.magnitude();
	geometry.scale.scale.y *= scaleMat.column1.magnitude();
	geometry.scale.scale.z *= scaleMat.column2.magnitude();

/*	// MONITOR: only allows for one kind of scaling
	geometry.scale.rotation = pose.q.getConjugate();
	geometry.scale.scale = geometry.scale.scale.multiply(scaling);
*/
	pose.p = pose.p.multiply(scaling);
}

} // namespace