#ifndef SCENEDOCUMENT_H
#define SCENEDOCUMENT_H

#include "breezEd.h"
#include "AbstractDocument.h"

#include <beScene/beResourceManager.h>
#include <beEntitySystem/beWorld.h>

#include <beEntitySystem/beSimulation.h>

#include <beScene/beEffectDrivenRenderer.h>
#include <beScene/beRenderContext.h>
#include <beScene/beSceneController.h>

#include <bePhysics/beScene.h>
#include <bePhysics/beSceneController.h>

#include <lean/smart/resource_ptr.h>

#include <QtCore/QVector>

#include <beCore/beParameters.h>

class QUndoStack;
class Interaction;
class DropInteraction;
class DeviceManager;

/// Scene document class
class SceneDocument : public AbstractDocument
{
	Q_OBJECT

public:
	/// Entity vector.
	typedef QVector<beEntitySystem::Entity*> EntityVector;

private:
	QUndoStack *m_pUndoStack;

	lean::resource_ptr<beEntitySystem::World> m_pWorld;

	lean::resource_ptr<beEntitySystem::Simulation> m_pSimulation;

	lean::resource_ptr<beScene::EffectDrivenRenderer> m_pRenderer;
	lean::resource_ptr<beScene::RenderContext> m_pRenderContext;
	lean::resource_ptr<beScene::SceneController> m_pScene;

	lean::resource_ptr<bePhysics::SceneController> m_pPhysics;

	QObject *m_pPrimaryView;

	QVector<Interaction*> m_interactions;
	DropInteraction *m_pDropInteraction;

	EntityVector m_selection;

public:
	/// Code document type name.
	static const char *const DocumentTypeName;

	/// Constructor.
	SceneDocument(const QString &type, const QString &name, const QString &file, bool bLoadFromFile, Editor *pEditor, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~SceneDocument();

	/// Sets the document name.
	virtual void setName(const QString &name);

	/// Saves the document using the given filename.
	virtual bool saveAs(const QString &file);

	/// Adds the given interaction.
	void pushInteraction(Interaction *pInteraction);
	/// Adds the given drop interaction.
	void pushInteraction(DropInteraction *pDropInteraction);
	/// Removes the given interaction.
	void removeInteraction(Interaction *pInteraction);
	/// Gets all interactions.
	const QVector<Interaction*>& interactions() const { return m_interactions; }
	/// Gets the current drop interaction, if set.
	DropInteraction *dropInteraction() const { return m_pDropInteraction; }

	/// Gets the selection.
	const EntityVector& selection() const { return m_selection; }

	/// Checks the primary view.
	bool isPrimary(QObject *pView) const { return (pView = m_pPrimaryView); }

	/// Undo stack.
	QUndoStack* undoStack() { return m_pUndoStack; }
	/// Undo stack.
	const QUndoStack* undoStack() const { return m_pUndoStack; }

	/// Gets all available serialization parameters.
	beCore::ParameterSet getSerializationParameters();
	/// Sets all available serialization parameters.
	void setSerializationParameters(beCore::ParameterSet &serializationParams);

	/// World.
	beEntitySystem::World* world() { return m_pWorld; }
	/// World.
	const beEntitySystem::World* world() const { return m_pWorld; }

	/// Simulation.
	beEntitySystem::Simulation* simulation() { return m_pSimulation; }
	/// Simulation.
	const beEntitySystem::Simulation* simulation() const { return m_pSimulation; }

	/// Renderer.
	beScene::EffectDrivenRenderer* renderer() { return m_pRenderer; }
	/// Renderer.
	const beScene::EffectDrivenRenderer* renderer() const { return m_pRenderer; }
	
	/// Scene.
	beScene::SceneController* scene() { return m_pScene; }
	/// Scene.
	const beScene::SceneController* scene() const { return m_pScene; }

	/// Device manager.
	DeviceManager* deviceManager() const;

public Q_SLOTS:
	/// Releases all references to foreign resources.
	void releaseReferences();

	/// Clears the selection.
	void clearSelection();
	/// Select everything.
	void selectAll();
	/// Sets the selection.
	void setSelection(beEntitySystem::Entity *pEntity);
	/// Sets the selection.
	void setSelection(const EntityVector &selection);

	/// Sets the primary view.
	void setPrimary(QObject *pView) { m_pPrimaryView = pView; }

Q_SIGNALS:
	/// Emitted whenever the selection has changed.
	void selectionChanged(SceneDocument *pDocument);
};

#endif
