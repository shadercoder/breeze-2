#ifndef SCALEINTERACTION_H
#define SCALEINTERACTION_H

#include "breezEd.h"
#include "Interaction.h"
#include <QtCore/QObject>

#include <beEntitySystem/beEntity.h>

class SceneDocument;
class ScaleEntityCommand;

/// Scale interaction.
class ScaleInteraction : public QObject, public Interaction
{
	Q_OBJECT

public:
	/// Entity vector.
	typedef QVector<beEntitySystem::Entity*> entity_vector;

private:
	SceneDocument *m_pDocument;

	lean::resource_ptr<beEntitySystem::Entity> m_axes[3];

	entity_vector m_selection;
	beMath::fvec3 m_centroid;

	ScaleEntityCommand *m_pCommand;
	beMath::fvec3 m_axisStop;
	uint4 m_axisID;

public:
	static const uint4 InvalidAxisID = static_cast<uint4>(-1);

	/// Constructor.
	ScaleInteraction(SceneDocument *pDocument, QObject *pParent = nullptr);
	/// Destructor.
	~ScaleInteraction();

	/// Steps the interaction.
	void step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective);

	/// Attaches this interaction.
	void attach();
	/// Detaches this interaction.
	void detach();

public Q_SLOTS:
	/// Finish editing.
	void finishEditing();
	/// Update widgets.
	void updateWidgets();
	/// Update widgets.
	void selectionChanged();
};

#endif
