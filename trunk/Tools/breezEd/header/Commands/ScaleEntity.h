#ifndef SCALEENTITYCOMMAND_H
#define SCALEENTITYCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <beEntitySystem/beEntities.h>
#include <vector>

class SceneDocument;

/// Scale entity command class.
class ScaleEntityCommand : public QUndoCommand
{
public:
	// Entity translation state.
	struct EntityState
	{
		beEntitySystem::Entity *pEntity;
		beMath::fvec3 prevScaling;
		beMath::fvec3 scaling;
		beMath::fvec3 prevPosition;
		beMath::fvec3 position;

		/// Constructor.
		explicit EntityState(beEntitySystem::Entity *pEntity)
			: pEntity(pEntity),
			prevScaling(pEntity->GetScaling()),
			scaling(prevScaling),
			prevPosition(pEntity->GetPosition()),
			position(prevPosition) { }

		/// Captures the current state.
		void capture()
		{
			scaling = pEntity->GetScaling();
			position = pEntity->GetPosition();
		}

		/// Applies the previous state.
		void undo() const
		{
			pEntity->SetScaling(prevScaling);
			pEntity->SetPosition(prevPosition);
			pEntity->NeedSync();
		}
		/// Applies the new state.
		void redo() const
		{
			pEntity->SetScaling(scaling);
			pEntity->SetPosition(position);
			pEntity->NeedSync();
		}
	};
	typedef std::vector<EntityState> entity_vector;

private:
	entity_vector m_entities;

public:
	/// Constructor.
	ScaleEntityCommand(const QVector<beEntitySystem::Entity*> &entities, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~ScaleEntityCommand();

	/// Captures the entities' current state.
	void capture();

	/// Resets the entities' transformation.
	void undo();
	/// Applies entities' the new transformation.
	void redo();
};

#endif
