#include "stdafx.h"
#include "Views/SceneView.h"

#include "Editor.h"
#include "DeviceManager.h"
#include <QtCore/QSettings>

#include "Documents/SceneDocument.h"
#include "Modes/Mode.h"
//#include "Modes/SceneViewModeState.h"

#include "Utility/InputProvider.h"

#include <beScene/bePipe.h>
#include <beScene/bePerspectivePool.h>
#include <beScene/beRenderingPipeline.h>
#include <beScene/beRenderer.h>
#include <beScene/beRenderContext.h>
#include <beGraphics/beTextureTargetPool.h>

#include "Interaction/Interaction.h"
#include "Interaction/DropInteraction.h"
#include "Interaction/FreeCamera.h"

#include <beScene/beProcessingPipeline.h>
#include <beScene/beQuadProcessor.h>

#include "Utility/Checked.h"

#include <lean/logging/errors.h>

namespace
{

/// Constructs a swap chain for the given editor.
lean::resource_ptr<beGraphics::SwapChain, true> createSwapChain(QWidget &widget, Editor &editor)
{
	const QSettings &settings = *editor.settings();

	beGraphics::SwapChainDesc desc;

	desc.Window = (HWND) widget.winId();
	desc.Windowed = true;

	desc.Display.Width = widget.width();
	desc.Display.Height = widget.height();
	desc.Display.Format = beGraphics::Format::R8G8B8A8U_SRGB;
	desc.Samples.Count = settings.value("graphicsDevice/multisampling", 0).toInt();
	
	return beGraphics::CreateSwapChain(*editor.deviceManager()->graphicsDevice(), desc);
}

/// Creates a camera for the given document.
beScene::CameraController* createCamera(beEntitySystem::Entity &entity, beScene::Renderer &renderer, SceneDocument &document)
{
	lean::resource_ptr<beScene::CameraController> pCamera = new_resource beScene::CameraController(document.scene());
	pCamera->SetPerspective( renderer.PerspectivePool()->GetPerspective(nullptr, nullptr, beScene::NormalPipelineStages) );
	entity.AddControllerKeep(pCamera);
	
	return pCamera;
}

/// Resizes the given swap chain.
void resizePipe(beScene::Pipe &pipe, beScene::RenderContext &context, beGraphics::SwapChain &chain, uint4 width, uint4 height)
{
	context.StateManager().ClearBindings();
	context.Context().ClearState();
	pipe.SetFinalTarget(nullptr);

	try
	{
		chain.Resize(width, height);
	}
	catch (...)
	{
		LEAN_LOG_ERROR_MSG("Resizing swap chain buffers failed");
	}

	pipe.SetFinalTarget(beGraphics::GetBackBuffer(chain).get());
}

/// Adds the given effect to the given processing pipeline
beg::Material* addEffect(const utf8_ntri &file, const utf8_ntri &args, beScene::ProcessingPipeline &pipeline,
	beScene::EffectDrivenRenderer &renderer, beScene::ResourceManager &resourceManager)
{
	lean::resource_ptr<beGraphics::Effect> processingEffect =
		resourceManager.EffectCache()->GetByFile(file, args, "");

	lean::resource_ptr<beg::Material> processingMaterial = beg::CreateMaterial(
			&processingEffect.get(), 1,
			*resourceManager.EffectCache()
		);

	lean::resource_ptr<beScene::QuadProcessor> processor = new_resource beScene::QuadProcessor(
			renderer.Device(),
			renderer.ProcessingDrivers()
		);
	processor->SetMaterial(processingMaterial);

	pipeline.Add(processor);

	return processingMaterial;
}

/// Sets the given swap chain for the given camera.
void setSwapChain(beScene::CameraController &camera, beGraphics::SwapChain *pSwapChain, SceneDocument &document)
{
	// Swap chain
	camera.GetPerspective()->SetPipe(
			beScene::CreatePipe(
					*beGraphics::GetBackBuffer(*pSwapChain),
					document.renderer()->TargetPool()
			).get()
		);

	// Tonemapping
	lean::resource_ptr<beScene::ProcessingPipeline> processing = lean::bind_resource( new beScene::ProcessingPipeline("PostProcessing") );

	// TODO: NO HARDCODED PATHS
	addEffect("Processing/SimpleTonemap.fx", "BE_LDR_PROCESSING", *processing, *document.renderer(), *document.graphicsResources());
	addEffect("Processing/FXAA.fx", "", *processing, *document.renderer(), *document.graphicsResources());
	
	camera.GetPerspective()->SetProcessor(processing);
}

} // namespace

// Constructor.
SceneView::SceneView(SceneDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: AbstractView(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pViewMode( new Mode(LEAN_ASSERT_NOT_NULL(pDocumentMode)) ),
	
	m_pInputProvider(),
	m_pCamera( m_pDocument->world()->Entities()->AddEntity("Camera", bees::Entities::AnonymousPersistentID) ),
	m_pCameraController( createCamera(*m_pCamera, *m_pDocument->renderer(), *m_pDocument) ),
	m_pCameraInteraction(),
	m_pDropInteraction()
{
	ui.setupUi(this);

	// View swap chain
	lean::resource_ptr<beGraphics::SwapChain> pSwapChain = createSwapChain(*ui.canvas, *m_pEditor);
	ui.canvas->setSwapChain(pSwapChain);

	// View camera
	setSwapChain(*m_pCameraController, pSwapChain, *m_pDocument);
	m_pCameraInteraction = new FreeCamera(m_pCamera.get(), this);

	// Track canvas input
	m_pInputProvider = new InputProvider(ui.canvas, ui.canvas);
	ui.canvas->installEventFilter(m_pInputProvider);

	// Connect with canvas
	checkedConnect(ui.canvas, SIGNAL(focussed(QWidget*)), this, SLOT(setPrimary()));
	checkedConnect(ui.canvas, SIGNAL(step(float)), this, SLOT(step(float)));
	checkedConnect(ui.canvas, SIGNAL(render()), this, SLOT(render()));
	checkedConnect(ui.canvas, SIGNAL(resized(int, int)), this, SLOT(canvasResized(int, int)));

	// Set up (default) view mode
//	m_pViewMode->addState( new SceneViewModeState(this, m_pEditor, m_pViewMode) );
	pDocumentMode->setDefaultChildMode(m_pViewMode);

	// ORDER: Be sure initialization has been successful
	m_pDocument->scene()->AddPerspective(m_pCameraController->GetPerspective());
}

// Destructor.
SceneView::~SceneView()
{
	// ORDER: Make sure perspective is removed straight away
	m_pDocument->scene()->RemovePerspective(m_pCameraController->GetPerspective());
}

// Activates this view.
void SceneView::activate()
{
	setPrimary();
}

// Makes this view the primary view.
void SceneView::setPrimary()
{
	m_pDocument->setPrimary(this);
}

// Steps the simulation.
void SceneView::step(float timeStep)
{
	if (m_pDocument->isPrimary(this))
	{
		m_pDocument->commit();
		m_pDocument->simulation()->Fetch();
		
		const QVector<Interaction*> &interactions = m_pDocument->interactions();

		// Interaction
		if (ui.canvas->hasFocus())
		{
			m_pInputProvider->updateMouseAsync();

			beScene::PerspectiveDesc perspectiveDesc = beScene::PerspectiveFromCamera(m_pCameraController);

			for (QVector<Interaction*>::const_iterator itInteraction = interactions.end(); itInteraction-- != interactions.begin(); )
				(*itInteraction)->step(timeStep, *m_pInputProvider, perspectiveDesc);

			// NOTE: Camera view-dependent, separate from document interactions
			m_pCameraInteraction->step(timeStep, *m_pInputProvider, perspectiveDesc);

			m_pEditor->setInfo(QString("(%1 %2 %3)").arg(m_pCamera->GetPosition()[0]).arg(m_pCamera->GetPosition()[1]).arg(m_pCamera->GetPosition()[2]));

			m_pInputProvider->step();
		}
		
		// Animation
		m_pDocument->simulation()->Step(timeStep);

		m_pDocument->simulation()->Flush();
	}
}

// Renders the scene.
void SceneView::render()
{
	if (m_pDocument->isPrimary(this))
	{
		m_pDocument->renderer()->InvalidateCaches();
		m_pDocument->simulation()->Render();

		// Get rid of unused targets
		m_pDocument->renderer()->TargetPool()->ReleaseUnused();
	}
}

// Canvas size changed.
void SceneView::canvasResized(int width, int height)
{
	m_pCameraController->SetAspect( (float) width / height );

	// Resize buffers & flush target pool (most sizes will have changed)
	resizePipe(*m_pCameraController->GetPerspective()->GetPipe(), *this->m_pDocument->scene()->GetRenderContext(), *ui.canvas->swapChain(), width, height);
	m_pDocument->renderer()->TargetPool()->ResetUsage();
}

// Handles drag&drop events.
void SceneView::dragEnterEvent(QDragEnterEvent *pEvent)
{
	DropInteraction *pDragDrop = interactionFromMimeData(pEvent->mimeData());

	// Pass on to drop interaction
	if (pDragDrop)
	{
		pDragDrop->accept(*pEvent);

		// IMPORTANT: Only accepted operations continue to be tracked
		if (pEvent->isAccepted())
		{
			m_pDocument->pushInteraction(pDragDrop);
			m_pDropInteraction = pDragDrop;
		}
	}

	ui.canvas->setFocus();
	setPrimary();
}

// Handles drag&drop events.
void SceneView::dragMoveEvent(QDragMoveEvent *pEvent)
{
	QWidget::dragMoveEvent(pEvent);

	m_pInputProvider->updateWhileDragging(this, pEvent);
}

// Handles drag&drop events.
void SceneView::dragLeaveEvent(QDragLeaveEvent *pEvent)
{
	// Pass on to drop interaction
	if (m_pDropInteraction)
	{
		m_pDropInteraction->cancel();
		m_pDocument->removeInteraction(m_pDropInteraction);
		m_pDropInteraction = nullptr;
	}

	// NOTE: We won't get any release events
	m_pInputProvider->release();
}

// Handles drag&drop events.
void SceneView::dropEvent(QDropEvent *pEvent)
{
	// Pass on to drop interaction
	if (m_pDropInteraction)
	{
		m_pDropInteraction->complete(*pEvent);
		m_pDocument->removeInteraction(m_pDropInteraction);
		m_pDropInteraction = nullptr;
	}

	// NOTE: We won't get any release events
	m_pInputProvider->release();
}
