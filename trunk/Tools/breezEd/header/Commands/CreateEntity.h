#ifndef CREATEENTITYCOMMAND_H
#define CREATEENTITYCOMMAND_H

#include <QtGui/QUndoCommand>

#include <beEntitySystem/beWorld.h>
#include <beEntitySystem/beEntity.h>
#include <lean/smart/resource_ptr.h>

#include <QtCore/QVector>

class SceneDocument;

/// Create entity command class.
class CreateEntityCommand : public QUndoCommand
{
public:
	typedef std::vector<lean::resource_ptr<beEntitySystem::Entity>> entity_vector;

private:
	SceneDocument *m_pDocument;

	lean::resource_ptr<beEntitySystem::World> m_pWorld;
	entity_vector m_entities;

	QVector<beEntitySystem::Entity*> m_prevSelection;

protected:
	CreateEntityCommand(const CreateEntityCommand&) { }
	CreateEntityCommand& operator =(const CreateEntityCommand&) { return *this; }

public:
	typedef lean::range<beEntitySystem::Entity *const *> EntityRange;

	/// Constructor.
	CreateEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld, beEntitySystem::Entity *pEntity, QUndoCommand *pParent = nullptr);
	/// Constructor.
	CreateEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld,
		beEntitySystem::Entity *const *entities, uint4 entityCount, const QString &name, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~CreateEntityCommand();

	/// Removes the created entity.
	void undo();
	/// Adds the created entity.
	void redo();
	
	/// Gets the entities.
	EntityRange entities() const { return EntityRange(&m_entities[0].get(), &m_entities[0].get() + m_entities.size()); }
};

#endif
