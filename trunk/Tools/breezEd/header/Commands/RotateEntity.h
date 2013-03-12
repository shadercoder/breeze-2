#ifndef ROTATEENTITYCOMMAND_H
#define ROTATEENTITYCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <beEntitySystem/beEntities.h>
#include <vector>

class SceneDocument;

/// Rotate entity command class.
class RotateEntityCommand : public QUndoCommand
{
public:
	// Entity translation state.
	struct EntityState
	{
		beEntitySystem::Entity *pEntity;
		beMath::fmat3 prevOrientation;
		beMath::fmat3 orientation;
		beMath::fvec3 prevPosition;
		beMath::fvec3 position;

		/// Constructor.
		explicit EntityState(beEntitySystem::Entity *pEntity)
			: pEntity(pEntity),
			prevOrientation(pEntity->GetOrientation()),
			orientation(prevOrientation),
			prevPosition(pEntity->GetPosition()),
			position(prevPosition) { }

		/// Captures the current state.
		void capture()
		{
			orientation = pEntity->GetOrientation();
			position = pEntity->GetPosition();
		}

		/// Applies the previous state.
		void undo() const
		{
			pEntity->SetOrientation(prevOrientation);
			pEntity->SetPosition(prevPosition);
			pEntity->NeedSync();
		}
		/// Applies the new state.
		void redo() const
		{
			pEntity->SetOrientation(orientation);
			pEntity->SetPosition(position);
			pEntity->NeedSync();
		}

	};
	typedef std::vector<EntityState> entity_vector;

private:
	entity_vector m_entities;

public:
	/// Constructor.
	RotateEntityCommand(const QVector<beEntitySystem::Entity*> &entities, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~RotateEntityCommand();

	/// Captures the entities' current state.
	void capture();

	/// Resets the entities' transformation.
	void undo();
	/// Applies entities' the new transformation.
	void redo();
};

#endif
