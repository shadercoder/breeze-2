#ifndef FREECAMERA_H
#define FREECAMERA_H

#include "breezEd.h"
#include "Interaction.h"
#include <QtCore/QObject>

#include <beEntitySystem/beEntity.h>

/// Free camera interaction.
class FreeCamera : public QObject, public Interaction
{
	Q_OBJECT

private:
	lean::resource_ptr<beEntitySystem::Entity> m_pEntity;

public:
	/// Constructor.
	FreeCamera(beEntitySystem::Entity *pEntity, QObject *pParent = nullptr);
	/// Destructor.
	~FreeCamera();

	/// Steps the interaction.
	void step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective);
};

#endif
