#ifndef TRANSLATEENTITYCOMMAND_H
#define TRANSLATEENTITYCOMMAND_H

#include <QtGui/QUndoCommand>

#include <beEntitySystem/beEntity.h>
#include <vector>

class SceneDocument;

/// Create entity command class.
class TranslateEntityCommand : public QUndoCommand
{
public:
	/// Entity translation state.
	struct EntityState
	{
		beEntitySystem::Entity *pEntity;
		beMath::fvec3 prevPosition;
		beMath::fvec3 position;

		/// Constructor.
		explicit EntityState(beEntitySystem::Entity *pEntity)
			: pEntity(pEntity),
			prevPosition(pEntity->GetPosition()),
			position(prevPosition) { }

		/// Captures the current state.
		void capture()
		{
			position = pEntity->GetPosition();
		}

		/// Applies the previous state.
		void undo() const
		{
			pEntity->SetPosition(prevPosition);
		}
		/// Applies the new state.
		void redo() const
		{
			pEntity->SetPosition(position);
		}
	};
	typedef std::vector<EntityState> entity_vector;

private:
	entity_vector m_entities;

protected:
	TranslateEntityCommand(const TranslateEntityCommand&) { }
	TranslateEntityCommand& operator =(const TranslateEntityCommand&) { return *this; }

public:
	/// Constructor.
	TranslateEntityCommand(const QVector<beEntitySystem::Entity*> &entities, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~TranslateEntityCommand();

	/// Captures the entities' current state.
	void capture();

	/// Resets the entities' transformation.
	void undo();
	/// Applies entities' the new transformation.
    void redo();
};

#endif
