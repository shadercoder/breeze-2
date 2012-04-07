/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_ENTITY
#define BE_ENTITYSYSTEM_ENTITY

#include "beEntitySystem.h"
#include <beCore/beShared.h>
#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>
#include <beCore/beExchangeContainers.h>
#include "beControllerDriven.h"
#include <beCore/beReflectionPropertyProvider.h>

namespace beEntitySystem
{

using namespace beMath::Types;

/// Serializable interface.
class Entity : public beCore::Resource, public ControllerDriven, public beCore::ReflectionPropertyProvider
{
private:
	utf8_string m_name;

	uint4 m_id;
	uint8 m_persistentID;

	ivec3 m_cell;
	fvec3 m_position;
	fmat3 m_orientation;
	fvec3 m_scaling;

protected:
	Entity& operator =(const Entity&) { return *this; }

public:
	/// Invalid ID.
	static const uint4 InvalidID = static_cast<uint4>(-1);

	/// Constructor.
	BE_ENTITYSYSTEM_API Entity(const utf8_ntri &name);
	/// Destructor.
	BE_ENTITYSYSTEM_API virtual ~Entity();

	/// Sets the cell.
	LEAN_INLINE void SetCell(const ivec3 &cell) { m_cell = cell; EmitPropertyChanged(); }
	/// Sets the (cell-relative) position.
	LEAN_INLINE void SetPosition(const fvec3 &position) { m_position = position; EmitPropertyChanged(); }
	/// Sets the orientation.
	LEAN_INLINE void SetOrientation(const fmat3 &orientation) { m_orientation = orientation; EmitPropertyChanged(); }
	/// Sets the scaling.
	LEAN_INLINE void SetScaling(const fvec3 &scaling) { m_scaling = scaling; EmitPropertyChanged(); }

	/// Gets the cell.
	LEAN_INLINE const ivec3& GetCell() const { return m_cell; }
	/// Gets the (cell-relative) position.
	LEAN_INLINE const fvec3& GetPosition() const { return m_position; }
	/// Gets the orientation.
	LEAN_INLINE const fmat3& GetOrientation() const { return m_orientation; }
	/// Gets the scaling.
	LEAN_INLINE const fvec3& GetScaling() const { return m_scaling; }

	/// Sets the orientation.
	BE_ENTITYSYSTEM_API void SetAngles(const fvec3 &angles);
	/// Gets the orientation.
	BE_ENTITYSYSTEM_API fvec3 GetAngles() const;

	/// Sets the name.
	BE_ENTITYSYSTEM_API void SetName(const utf8_ntri &name);
	/// Gets the name.
	LEAN_INLINE const utf8_string& GetName() const { return m_name; }

	/// Sets the entity ID.
	LEAN_INLINE void SetID(uint4 entityID) { m_id = entityID; }
	/// Gets the entity ID.
	LEAN_INLINE uint4 GetID() const { return m_id; }

	/// Sets the persistent ID.
	LEAN_INLINE void SetPersistentID(uint8 persistentID) { m_persistentID = persistentID; }
	/// Gets the persistent ID.
	LEAN_INLINE uint8 GetPersistentID() const { return m_persistentID; }

	/// Gets the reflection properties.
	BE_ENTITYSYSTEM_API static Properties GetEntityProperties();
	/// Gets the reflection properties.
	BE_ENTITYSYSTEM_API Properties GetReflectionProperties() const;

	/// Gets the entity type.
	BE_ENTITYSYSTEM_API static utf8_ntr GetEntityType();
	/// Gets the entity type.
	utf8_ntr GetType() const { return GetEntityType(); }
};

} // namespace

#endif