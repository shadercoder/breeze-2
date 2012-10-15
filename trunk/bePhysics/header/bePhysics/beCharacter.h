/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_CHARACTER
#define BE_PHYSICS_CHARACTER

#include "bePhysics.h"
#include <beCore/beShared.h>
#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>
#include <lean/smart/resource_ptr.h>

namespace bePhysics
{

/// Character collision flags.
namespace CharacterCollision
{
	/// Enumeration.
	enum T
	{
		Down = 1 << 0,		///< Collisions below.
		Sides = 1 << 1,		///< Wall collisions.
		Up = 1 << 2			///< Collisions above.
	};
}
/// Collision flags.
typedef uint4 CharacterCollisionFlags;

/// Character interface.
class Character : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE Character& operator =(const Character&) { return *this; }

public:
	virtual ~Character() throw() { }

	/// Moves the character by the given amount.
	virtual CharacterCollisionFlags Move(const beMath::fvec3 &move, float minDist = 0.00001f) = 0;

	/// Checks if the given shape would currently overlap.
	virtual bool CheckShape(float radius, float height) const = 0;
	/// Checks if the given shape would currently overlap.
	virtual bool CheckShape(const beMath::fvec3 &pos, float radius, float height) const = 0;

	/// Changes the shape.
	virtual void SetShape(float radius, float height) = 0;
	/// Gets the current radius.
	virtual float GetRadius() const = 0;
	/// Gets the current height.
	virtual float GetHeight() const = 0;

	/// Sets the position.
	virtual void SetPosition(const beMath::fvec3 &pos) = 0;
	/// Gets the current position.
	virtual beMath::fvec3 GetPosition() const = 0;

	/// Gets the current slope limit.
	virtual float GetSlopeLimit() const = 0;

	/// Sets the step offset.
	virtual void SetStepOffset(float offset) = 0;
	/// Gets the current step offset.
	virtual float GetStepOffset() const = 0;
};

/// Character callback interface.
class CharacterCallback;

/// Character description.
struct CharacterDesc
{
	float Radius;		///< Character radius.
	float Height;		///< Character height.

	float SlopeLimit;	///< Maximum slope angle.
	float StepOffset;	///< Maximum change in ground height.
	float SkinWidth;	///< Contact tolerance.

	/// Constructor.
	CharacterDesc(float radius = 0.4f,
			float height = 1.0f,
			float slopeLimit = 0.5f,
			float stepOffset = 0.5f,
			float skinWidth = 0.1f)
		: Radius(radius),
		Height(height),
		SlopeLimit(slopeLimit),
		StepOffset(stepOffset),
		SkinWidth(skinWidth) { }

};

class Scene;
class CharacterScene;
class Material;

/// Creates a character from the given capsule.
BE_PHYSICS_API lean::resource_ptr<Character, true> CreateCharacter(CharacterScene *charScene, Scene *scene,
	const CharacterDesc &desc, const Material *material, CharacterCallback *pCallback = nullptr);

/// Sets filter data for the given character.
BE_PHYSICS_API void SetSimulationFilterData(Character &character, uint4 groupFlags, uint4 typeFlags, uint4 ID);
/// Gets filter data from the given character.
BE_PHYSICS_API void GetSimulationFilterData(const Character &character, int4 &groupFlags, uint4 &typeFlags, uint4 &ID);

} // namespace

#endif