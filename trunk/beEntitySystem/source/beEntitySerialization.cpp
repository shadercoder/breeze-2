/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beEntitySerialization.h"
#include "beEntitySystem/beEntity.h"
#include "beEntitySystem/beEntitySerializer.h"

namespace beEntitySystem
{
	// Instantiate entity serialization
	template class Serialization<Entity, EntitySerializer>;
}
// Link entity serialization
#include "beSerialization.cpp"

namespace beEntitySystem
{

// Gets the entity serialization register.
EntitySerialization& GetEntitySerialization()
{
	static EntitySerialization manager;
	return manager;
}

} // namespace
