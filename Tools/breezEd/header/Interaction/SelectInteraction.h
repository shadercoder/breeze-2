#ifndef SELECTINTERACTION_H
#define SELECTINTERACTION_H

#include "breezEd.h"
#include "Interaction.h"
#include <QtCore/QObject>

class SceneDocument;

/// Free camera interaction.
class SelectInteraction : public QObject, public Interaction
{
	Q_OBJECT

private:
	SceneDocument *m_pDocument;

public:
	/// Constructor.
	SelectInteraction(SceneDocument *pDocument, QObject *pParent = nullptr);
	/// Destructor.
	~SelectInteraction();

	/// Steps the interaction.
	void step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective);
};

#endif
