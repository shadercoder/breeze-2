#include "stdafx.h"
#include "Commands/RemoveEntity.h"

#include "Documents/SceneDocument.h"

#include <QtCore/QCoreApplication>
#include "Utility/Strings.h"

#include <beEntitySystem/beEntityController.h>

#include <lean/functional/algorithm.h>

// Constructor.
RemoveEntityCommand::RemoveEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld, beEntitySystem::Entity *pEntity, QUndoCommand *pParent)
	: QUndoCommand(
			QCoreApplication::translate("RemoveEntityCommand", "Removed entity '%1'").arg( makeName(toQt(pEntity->GetName())) ),
			pParent
		),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pWorld( LEAN_ASSERT_NOT_NULL(pWorld) ),
	m_prevSelection( m_pDocument->selection() )
{
	LEAN_ASSERT_NOT_NULL( pEntity );
	LEAN_ASSERT( MayBeRemoved(&pEntity, 1, false) );

	m_entities.push_back(pEntity);
}

// Constructor.
RemoveEntityCommand::RemoveEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld,
		beEntitySystem::Entity *const *entities, uint4 entityCount, QUndoCommand *pParent)
	: QUndoCommand(
			QCoreApplication::translate("RemoveEntityCommand", "Removed entities '%1'+").arg( makeName(toQt(entities[0]->GetName())) ),
			pParent
		),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pWorld( LEAN_ASSERT_NOT_NULL(pWorld) ),
	m_entities( entities, entities + entityCount ),
	m_prevSelection( m_pDocument->selection() )
{
	LEAN_ASSERT( MayBeRemoved(entities, entityCount, false) );
}

// Destructor.
RemoveEntityCommand::~RemoveEntityCommand()
{
}

// Removes the created entity.
void RemoveEntityCommand::undo()
{
	for (entity_vector::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		if (it->owner)
			it->entity->SetOwner(it->owner);
		it->entity->Attach();
	}

	m_pDocument->setSelection(m_prevSelection);
}

// Adds the created entity.
void RemoveEntityCommand::redo()
{
	QVector<beEntitySystem::Entity*> selection = m_pDocument->selection();
	lean::remove_all(selection, m_entities.data(), m_entities.data() + m_entities.size());
	m_pDocument->setSelection(selection);

	for (entity_vector::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		it->entity->Detach();
		if (it->owner)
			it->entity->SetOwner(nullptr);
	}
}

// Checks if the given list may be removed.
bool RemoveEntityCommand::MayBeRemoved(beEntitySystem::Entity *const *entities, uint4 entityCount, bool bShowError)
{
	bool mayBeRemoved = true;

	for (uint4 i = 0; i < entityCount; ++i)
		if (bees::EntityController *owner = entities[i]->GetOwner())
			if ((owner->GetChildFlags() & bees::ChildEntityFlags::OpenGroup) == 0)
			{
				mayBeRemoved = false;
				QMessageBox::critical(nullptr,
						QCoreApplication::translate("RemoveEntityCommand", "Unable to remove entities"),
						QCoreApplication::translate("RemoveEntityCommand", "The selected group of entities contains one or more entities owned by closed groups.")
					);
				break;
			}

	return mayBeRemoved;
}
