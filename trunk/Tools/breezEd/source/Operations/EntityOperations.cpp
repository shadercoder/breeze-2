#include "stdafx.h"
#include "Operations/EntityOperations.h"

#include "Documents/SceneDocument.h"

#include "Commands/CreateEntity.h"

// Duplicates the given set of entities.
const CreateEntityCommand* DuplicateEntities(const beEntitySystem::Entity *const *entities, uint4 entityCount, SceneDocument *document)
{
	lean::scoped_ptr<lean::scoped_ptr<bees::Entity>[]> copies = new_scoped_array lean::scoped_ptr<bees::Entity>[entityCount];

	for (uint4 i = 0; i < entityCount; ++i)
		copies[i] = entities[i]->Clone();

	lean::scoped_ptr<CreateEntityCommand> command( new CreateEntityCommand(document, document->world(), &copies[0].get(), entityCount, "Clone") );
	copies.detach();
	
	document->undoStack()->push(command);
	return command.detach();
}
