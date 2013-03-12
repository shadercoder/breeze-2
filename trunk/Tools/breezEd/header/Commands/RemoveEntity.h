#ifndef REMOVEENTITYCOMMAND_H
#define REMOVEENTITYCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <beEntitySystem/beWorld.h>
#include <beEntitySystem/beEntities.h>
#include <lean/smart/resource_ptr.h>

#include <QtCore/QVector>

class SceneDocument;

/// Create entity command class.
class RemoveEntityCommand : public QUndoCommand
{
public:
	struct Removed
	{
		beEntitySystem::Entity *entity;
		beEntitySystem::EntityController *owner;

		Removed(beEntitySystem::Entity *entity)
			: entity(entity),
			owner(entity->GetOwner()) { }

		friend bool operator ==(const Removed &a, beEntitySystem::Entity *b) { return a.entity == b; }
		friend bool operator ==(beEntitySystem::Entity *a, const Removed &b) { return a == b.entity; }
	};

	typedef std::vector<Removed> entity_vector;

private:
	SceneDocument *m_pDocument;

	lean::resource_ptr<beEntitySystem::World> m_pWorld;
	entity_vector m_entities;

	QVector<beEntitySystem::Entity*> m_prevSelection;

public:
	typedef lean::range<Removed const *> EntityRange;

	/// Constructor.
	RemoveEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld, beEntitySystem::Entity *pEntity, QUndoCommand *pParent = nullptr);
	/// Constructor.
	RemoveEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld,
		beEntitySystem::Entity *const *entities, uint4 entityCount, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~RemoveEntityCommand();

	/// Removes the created entity.
	void undo();
	/// Adds the created entity.
	void redo();
	
	/// Gets the entities.
	EntityRange entities() const { return EntityRange(&m_entities[0], &m_entities[0] + m_entities.size()); }

	/// Checks if the given list may be removed.
	static bool MayBeRemoved(beEntitySystem::Entity *const *entities, uint4 entityCount, bool bShowError);
};

#endif
