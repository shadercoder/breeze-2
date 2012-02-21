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
private:
	SceneDocument *m_pDocument;
	lean::resource_ptr<beEntitySystem::World> m_pWorld;
	lean::resource_ptr<beEntitySystem::Entity> m_pEntity;

	QVector<beEntitySystem::Entity*> m_prevSelection;

protected:
	CreateEntityCommand(const CreateEntityCommand&) { }
	CreateEntityCommand& operator =(const CreateEntityCommand&) { return *this; }

public:
	/// Constructor.
	CreateEntityCommand(SceneDocument *pDocument, beEntitySystem::World *pWorld, beEntitySystem::Entity *pEntity, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~CreateEntityCommand();

	/// Removes the created entity.
	void undo();
	/// Adds the created entity.
    void redo();

	/// Entity.
	beEntitySystem::Entity* entity() const { return m_pEntity; }
	/// World.
	beEntitySystem::World* world() const { return m_pWorld; }
};

#endif
