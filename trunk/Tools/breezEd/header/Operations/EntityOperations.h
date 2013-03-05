#ifndef ENTITYOPERATIONS_H
#define ENTITYOPERATIONS_H

#include "breezEd.h"

#include <beEntitySystem/beEntities.h>

class SceneDocument;
class CreateEntityCommand;

/// Duplicates the given set of entities.
const CreateEntityCommand* DuplicateEntities(const beEntitySystem::Entity *const *entities, uint4 entityCount, SceneDocument *document);

#endif
