/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_WORLD
#define BE_ENTITYSYSTEM_WORLD

#include "beEntitySystem.h"
#include <lean/tags/noncopyable.h>
#include <beCore/beShared.h>
#include <lean/smart/resource_ptr.h>
#include <vector>
#include <beCore/bePersistentIDs.h>
#include <lean/rapidxml/rapidxml.hpp>

// Prototypes
namespace beCore
{
	class ParameterSet;
}

namespace beEntitySystem
{

// Prototypes
class Entity;

/// World description.
struct WorldDesc
{
	int4 CellSize;		///< Size of one world cell.

	/// Constructor.
	WorldDesc(int4 cellSize = 10000)
		: CellSize(cellSize) { }
};

/// World class.
class World : public lean::noncopyable_chain<beCore::Resource>
{
private:
	utf8_string m_name;

	typedef std::vector< lean::resource_ptr<Entity> > entity_vector;
	entity_vector m_entities;

	beCore::PersistentIDs m_persistentIDs;

	WorldDesc m_desc;

	/// Saves the world to the given xml node.
	void SaveWorld(rapidxml::xml_node<lean::utf8_t> &node) const;
	/// Loads the world from the given xml node.
	void LoadWorld(const rapidxml::xml_node<lean::utf8_t> &node, beCore::ParameterSet &parameters);

public:
	/// Entity range.
	typedef lean::range<Entity *const *> Entities;
	/// Const entity range.
	typedef lean::range<const Entity *const *> ConstEntities;

	/// Creates an empty world.
	BE_ENTITYSYSTEM_API explicit World(const utf8_ntri &name, const WorldDesc &desc = WorldDesc());
	/// Loads the world from the given file.
	BE_ENTITYSYSTEM_API explicit World(const utf8_ntri &name, const utf8_ntri &file, beCore::ParameterSet &parameters, const WorldDesc &desc = WorldDesc());
	/// Loads the world from the given XML node.
	BE_ENTITYSYSTEM_API explicit World(const utf8_ntri &name, const rapidxml::xml_node<lean::utf8_t> &node, beCore::ParameterSet &parameters, const WorldDesc &desc = WorldDesc());
	/// Destructor.
	BE_ENTITYSYSTEM_API virtual ~World();

	/// Adds the given entity to this world.
	BE_ENTITYSYSTEM_API void AddEntity(Entity *pEntity, bool bKeepPersistentId = false);
	/// Removes the given entity from this world.
	BE_ENTITYSYSTEM_API bool RemoveEntity(Entity *pEntity, bool bKeepPersistentId = false);

	/// Gets the number of entities.
	BE_ENTITYSYSTEM_API uint4 GetEntityCount() const;
	/// Gets the n-th entity.
	LEAN_INLINE Entity* GetEntity(uint4 id)
	{
		return const_cast<Entity*>( const_cast<const World*>(this)->GetEntity(id) );
	}
	/// Gets the n-th entity.
	BE_ENTITYSYSTEM_API const Entity* GetEntity(uint4 id) const;

	/// Gets a range of all entities.
	LEAN_INLINE Entities GetEntities()
	{
		return Entities( &m_entities[0].get(), &m_entities[0].get() + m_entities.size() );
	}
	/// Gets a range of all entities.
	LEAN_INLINE ConstEntities GetEntities() const
	{
		return ConstEntities( &m_entities[0].get(), &m_entities[0].get() + m_entities.size() );
	}

	/// Attaches all entities to their simulations.
	BE_ENTITYSYSTEM_API void Attach() const;
	/// Detaches all entities from their simulations.
	BE_ENTITYSYSTEM_API void Detach() const;

	/// Saves the world to the given file.
	BE_ENTITYSYSTEM_API void Serialize(const lean::utf8_ntri &file) const;
	/// Saves the world to the given XML node.
	BE_ENTITYSYSTEM_API void Serialize(rapidxml::xml_node<lean::utf8_t> &node) const;

	/// Gets the world's persistent IDs.
	LEAN_INLINE beCore::PersistentIDs& persistentIDs() { return m_persistentIDs; }
	/// Gets the world's persistent IDs.
	LEAN_INLINE const beCore::PersistentIDs& persistentIDs() const { return m_persistentIDs; }

	/// Gets the world's cell size.
	LEAN_INLINE int4 GetCellSize() { return m_desc.CellSize; }

	/// Sets the name.
	BE_ENTITYSYSTEM_API void SetName(const utf8_ntri &name);
	/// Gets the name.
	LEAN_INLINE const utf8_string& GetName() const { return m_name; }
};

}

#endif