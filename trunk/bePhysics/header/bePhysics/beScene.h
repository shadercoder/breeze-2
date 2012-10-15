/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_SCENE
#define BE_PHYSICS_SCENE

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <lean/smart/resource_ptr.h>
#include <beMath/beVectorDef.h>

namespace bePhysics
{

/// Physics scene description.
struct SceneDesc
{
	beMath::fvec3 Gravity;			///< Scene gravity.
	float BounceThresholdSpeed;		///< Prevents bouncing below the specified velocity.
	float ContactCorrelationDist;	///< Contact merging tolerance.

	/// Constructor.
	SceneDesc(const beMath::fvec3 &gravity = beMath::vec(0.0f, -9.81f, 0.0f),
		float bounceThresholdSpeed = 0.0f,
		float contactCorrelationDist = 0.0f)
			: Gravity(gravity),
			BounceThresholdSpeed(bounceThresholdSpeed),
			ContactCorrelationDist(contactCorrelationDist) { }
};

/// Physics scene interface.
class Scene : public beCore::Resource, public Implementation
{
protected:
	LEAN_INLINE Scene& operator =(const Scene&) { return *this; }

public:
	virtual ~Scene() throw() { }
};

// Prototypes
class Device;

/// Creates a physics scene.
BE_PHYSICS_API lean::resource_ptr<Scene, true> CreateScene(Device *device, const SceneDesc &desc);

} // namespace

#endif