#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include "breezEd.h"
#include "AbstractView.h"
#include "ui_SceneView.h"

#include <beEntitySystem/beEntity.h>
#include <beScene/beCameraController.h>

#include <lean/smart/scoped_ptr.h>
#include <lean/smart/resource_ptr.h>

class Editor;
class SceneDocument;
class Mode;
class InputProvider;
class FreeCamera;

class SceneView : public AbstractView
{
	Q_OBJECT

private:
	Ui::SceneView ui;

	Editor *m_pEditor;

	SceneDocument *m_pDocument;

	Mode *m_pViewMode;

	InputProvider *m_pInputProvider;

	lean::resource_ptr<beEntitySystem::Entity> m_pCamera;
	lean::resource_ptr<beScene::CameraController> m_pCameraController;
	FreeCamera *m_pCameraInteraction;

protected:
	/// Handles drag&drop events.
	void dragEnterEvent(QDragEnterEvent *pEvent);
	/// Handles drag&drop events.
	void dragMoveEvent(QDragMoveEvent *pEvent);
	/// Handles drag&drop events.
	void dragLeaveEvent(QDragLeaveEvent *pEvent);
	/// Handles drag&drop events.
	void dropEvent(QDropEvent *pEvent);

public:
	/// Constructor.
	SceneView(SceneDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~SceneView();

	/// Activates this view.
	void activate();

public Q_SLOTS:
	/// Canvas size changed.
	void canvasResized(int width, int height);

	/// Steps the simulation.
	void step(float timeStep);
	/// Renders the scene.
	void render();

	/// Makes this view the primary view.
	void setPrimary();
};

#endif