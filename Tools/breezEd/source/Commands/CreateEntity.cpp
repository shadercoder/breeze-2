#include "stdafx.h"
#include "Commands/CreateEntity.h"

#include "Documents/SceneDocument.h"

#include <QtCore/QCoreApplication>
#include "Utility/Strings.h"

// Constructor.
CreateEntityCommand::CreateEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld, beEntitySystem::Entity *pEntity, QUndoCommand *pParent)
	: QUndoCommand(
		QCoreApplication::translate("CreateEntityCommand", "Created entity '%1'").arg( makeName(toQt(pEntity->GetName())) ),
		pParent ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pWorld( LEAN_ASSERT_NOT_NULL(pWorld) ),
	m_pEntity( LEAN_ASSERT_NOT_NULL(pEntity) ),
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
	m_pWorld->RemoveEntity(m_pEntity, true);
	m_pEntity->Detach();

	m_pDocument->setSelection(m_prevSelection);
}

// Adds the created entity.
void CreateEntityCommand::redo()
{
	m_pWorld->AddEntity(m_pEntity, true);
	m_pEntity->Attach();

	m_pDocument->setSelection(m_pEntity);
}
