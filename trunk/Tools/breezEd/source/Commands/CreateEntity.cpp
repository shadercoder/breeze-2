#include "stdafx.h"
#include "Commands/CreateEntity.h"

#include "Documents/SceneDocument.h"

#include <QtCore/QCoreApplication>
#include "Utility/Strings.h"

// Constructor.
CreateEntityCommand::CreateEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld, beEntitySystem::Entity *pEntity, QUndoCommand *pParent)
	: QUndoCommand(
			QCoreApplication::translate("CreateEntityCommand", "Created entity '%1'").arg( makeName(toQt(pEntity->GetName())) ),
			pParent
		),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pWorld( LEAN_ASSERT_NOT_NULL(pWorld) ),
	m_prevSelection( m_pDocument->selection() )
{
	m_entities.push_back( LEAN_ASSERT_NOT_NULL(pEntity) );
}

// Constructor.
CreateEntityCommand::CreateEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld,
		beEntitySystem::Entity *const *entities, uint4 entityCount, const QString &name, QUndoCommand *pParent)
	: QUndoCommand(
			QCoreApplication::translate("CreateEntityCommand", "Created asset '%1'").arg( makeName(name) ),
			pParent
		),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pWorld( LEAN_ASSERT_NOT_NULL(pWorld) ),
	m_entities( entities, entities + entityCount ),
	m_prevSelection( m_pDocument->selection() )
{
}

// Destructor.
CreateEntityCommand::~CreateEntityCommand()
{
}

// Removes the created entity.
void CreateEntityCommand::undo()
{
	for (entity_vector::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
		(*it)->Detach();

	m_pDocument->setSelection(m_prevSelection);
}

// Adds the created entity.
void CreateEntityCommand::redo()
{
	QVector<beEntitySystem::Entity*> selection;
	selection.reserve( static_cast<int>(m_entities.size()) );

	for (entity_vector::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		(*it)->Attach();
		selection.push_back(*it);
	}

	m_pDocument->setSelection(selection);
}
