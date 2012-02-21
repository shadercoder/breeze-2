#include "stdafx.h"
#include "Commands/ScaleEntity.h"

#include <QtCore/QCoreApplication>
#include "Utility/Strings.h"

namespace
{

/// Constructs a command name from the given entities.
QString makeCommandName(const QVector<beEntitySystem::Entity*> &entities)
{
	return (entities.size() == 1)
		? QCoreApplication::translate("ScaleEntityCommand", "Scaled entity '%1'").arg( makeName( toQt(entities.front()->GetName()) ) )
		: QCoreApplication::translate("ScaleEntityCommand", "Scaled entities");
}

} // namespace


// Constructor.
ScaleEntityCommand::ScaleEntityCommand(const QVector<beEntitySystem::Entity*> &entities, QUndoCommand *pParent)
	: QUndoCommand( makeCommandName(entities), pParent )
{
	m_entities.reserve( entities.size() );

	Q_FOREACH (beEntitySystem::Entity *pEntity, entities)
		m_entities.push_back( EntityState(pEntity) );
}

// Destructor.
ScaleEntityCommand::~ScaleEntityCommand()
{
}

// Captures the entities' current state.
void ScaleEntityCommand::capture()
{
	for (entity_vector::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
		it->capture();
}

// Resets the entities' transformation.
void ScaleEntityCommand::undo()
{
	for (entity_vector::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
		it->undo();
}

// Applies entities' the new transformation.
void ScaleEntityCommand::redo()
{
	for (entity_vector::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
		it->redo();
}
