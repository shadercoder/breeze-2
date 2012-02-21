/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_ENTITYSERIALIZATION
#define BE_ENTITYSYSTEM_ENTITYSERIALIZATION

#include "beEntitySystem.h"
#include "beSerialization.h"

namespace beEntitySystem
{

class Entity;
class EntitySerializer;

/// Entity serialization.
typedef Serialization<Entity, EntitySerializer> EntitySerialization;

/// Gets the entity serialization register.
BE_ENTITYSYSTEM_API EntitySerialization& GetEntitySerialization();

}

#endif