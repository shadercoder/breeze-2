#ifndef ENTITYOPERATIONS_H
#define ENTITYOPERATIONS_H

#include "breezEd.h"

#include <beEntitySystem/beEntities.h>

class SceneDocument;
class CreateEntityCommand;
class RemoveEntityCommand;

/// Duplicates the given set of entities.
const CreateEntityCommand* DuplicateEntities(const beEntitySystem::Entity *const *entities, uint4 entityCount, SceneDocument *document);
/// Clones all selected entities.
const CreateEntityCommand* DuplicateSelection(SceneDocument *document);
/// Removes the given set of entities.
const RemoveEntityCommand* RemoveEntities(beEntitySystem::Entity *const *entities, uint4 entityCount, SceneDocument *document);
/// Removes the given set of entities.
const RemoveEntityCommand* RemoveSelection(SceneDocument *document);

#endif
