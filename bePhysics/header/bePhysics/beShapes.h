/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_SHAPES
#define BE_PHYSICS_SHAPES

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Shape compound interface.
class ShapeCompound : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE ShapeCompound& operator =(const ShapeCompound&) { return *this; }

public:
	virtual ~ShapeCompound() throw() { }
};

// Prototypes
class Device;
class Material;

/// Creates a shape from the given shape file.
BE_PHYSICS_API lean::resource_ptr<ShapeCompound, true> LoadShape(const utf8_ntri &file, const Material *pMaterial, Device &device);

} // namespace

#endif