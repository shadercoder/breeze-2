#include "stdafx.h"
#include "Operations/EntityOperations.h"

#include "Documents/SceneDocument.h"

#include "Commands/CreateEntity.h"
#include "Commands/RemoveEntity.h"

// Duplicates the given set of entities.
const CreateEntityCommand* DuplicateEntities(const beEntitySystem::Entity *const *entities, uint4 entityCount, SceneDocument *document)
{
	if (!entities || entityCount == 0)
		return nullptr;

	lean::scoped_ptr<lean::scoped_ptr<bees::Entity>[]> copies = new_scoped_array lean::scoped_ptr<bees::Entity>[entityCount];

	for (uint4 i = 0; i < entityCount; ++i)
		copies[i] = entities[i]->Clone();

	lean::scoped_ptr<CreateEntityCommand> command( new CreateEntityCommand(document, document->world(), &copies[0].get(), entityCount, "Clone") );
	copies.detach();
	
	document->undoStack()->push(command);
	return command.detach();
}

// Clones all selected entities.
const CreateEntityCommand* DuplicateSelection(SceneDocument *document)
{
	QVector<bees::Entity*> selection = document->selection();
	return DuplicateEntities(selection.data(), (uint4) selection.size(), document);
}

// Removes the given set of entities.
const RemoveEntityCommand* RemoveEntities(beEntitySystem::Entity *const *entities, uint4 entityCount, SceneDocument *document)
{
	if (!entities || entityCount == 0)
		return nullptr;

	if (!RemoveEntityCommand::MayBeRemoved(entities, entityCount, true))
		return nullptr;

	lean::scoped_ptr<RemoveEntityCommand> command( new RemoveEntityCommand(document, document->world(), entities, entityCount) );
	document->undoStack()->push(command);
	return command.detach();
}

// Removes all selected entities.
const RemoveEntityCommand* RemoveSelection(SceneDocument *document)
{
	QVector<bees::Entity*> selection = document->selection();
	return RemoveEntities(selection.data(), (uint4) selection.size(), document);
}
