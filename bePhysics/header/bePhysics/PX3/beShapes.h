/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_SHAPES_PX
#define BE_PHYSICS_SHAPES_PX

#include "bePhysics.h"
#include "../beShapes.h"
#include <lean/tags/noncopyable.h>
#include "beAPI.h"
#include <vector>

namespace bePhysics
{

// Prototypes
class Device;
class ShapeCache;

namespace PX3
{

/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxShape &shape, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxBoxGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxSphereGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxPlaneGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxCapsuleGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxConvexMeshGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);
/// Scales the given shape as precisely as possible.
BE_PHYSICS_PX_API void Scale(physx::PxTriangleMeshGeometry &geometry, physx::PxTransform &location, const physx::PxVec3 &scaling);

/// Shape compound implementation.
class ShapeCompound : public lean::noncopyable, public bePhysics::ShapeCompound
{
public:
	template <class Geometry>
	struct ShapeDesc
	{
		Geometry geom;						///< Shape geometry.
		const physx::PxMaterial *mat;		///< Shape material.
		physx::PxTransform pose;			///< Shape pose.

		/// Constructor.
		ShapeDesc(const Geometry &geom, const physx::PxMaterial *mat, const physx::PxTransform &pose)
			: geom(geom),
			mat(mat),
			pose(pose) { }
	};

	typedef ShapeDesc<physx::PxBoxGeometry> box_shape;
	typedef ShapeDesc<physx::PxSphereGeometry> sphere_shape;
	typedef ShapeDesc<physx::PxConvexMeshGeometry> convex_shape;
	typedef ShapeDesc<physx::PxTriangleMeshGeometry> mesh_shape;

	typedef std::vector<box_shape> box_vector;
	typedef std::vector<sphere_shape> sphere_vector;
	typedef std::vector<convex_shape> convex_vector;
	typedef std::vector<mesh_shape> mesh_vector;
	
	typedef std::vector<physx::PxGeometry*> geometry_vector;
	typedef std::vector<const physx::PxMaterial*> material_vector;
	typedef std::vector<physx::PxTransform> pose_vector;

private:
	box_vector m_boxes;
	sphere_vector m_spheres;
	convex_vector m_convexes;
	mesh_vector m_meshes;

	geometry_vector m_geometry;
	material_vector m_materials;
	pose_vector m_poses;

	bePhysics::ShapeCache *m_pShapeCache;

	/// Rebuilds the geometry vector.
	void RebuildGeometry();

public:
	/// Constructor.
	BE_PHYSICS_PX_API ShapeCompound(bePhysics::ShapeCache *pShapeCache = nullptr);
	/// Destructor.
	BE_PHYSICS_PX_API ~ShapeCompound();

	/// Adds the given box.
	BE_PHYSICS_PX_API void AddShape(const physx::PxBoxGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose);
	/// Adds the given sphere.
	BE_PHYSICS_PX_API void AddShape(const physx::PxSphereGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose);
	/// Adds the given convex mesh.
	BE_PHYSICS_PX_API void AddShape(const physx::PxConvexMeshGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose);
	/// Adds the given triangle mesh.
	BE_PHYSICS_PX_API void AddShape(const physx::PxTriangleMeshGeometry &shape, const physx::PxMaterial *pMaterial, const physx::PxTransform &pose);

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return PX3Implementation; }

	/// Gets the PhysX shapes.
	LEAN_INLINE const physx::PxGeometry *const * GetGeometry() const { return &m_geometry[0]; }
	/// Gets the PhysX shapes.
	LEAN_INLINE const physx::PxMaterial *const * GetMaterials() const { return &m_materials[0]; }
	/// Gets the PhysX shapes.
	LEAN_INLINE const physx::PxTransform* GetPoses() const { return &m_poses[0]; }
	/// Gets the number of shapes.
	LEAN_INLINE uint4 GetShapeCount() const { return static_cast<uint4>( m_geometry.size() ); }

	/// Gets the shape cache.
	bePhysics::ShapeCache* GetCache() const { return m_pShapeCache; }
};

template <> struct ToImplementationPX<bePhysics::ShapeCompound> { typedef ShapeCompound Type; };

} // namespace

} // namespace

#endif