#include "stdafx.h"
#include "Tiles/EntityBuilderWidget.h"

#include "Documents/SceneDocument.h"

#include "Widgets/ControllerBuilderWidget.h"

#include <beEntitySystem/beControllerSerializer.h>
#include <beEntitySystem/beControllerSerialization.h>

#include <QtCore/QSignalMapper>
#include <QtGui/QDrag>
#include "Interaction/DropInteraction.h"

#include <beCore/beParameterSet.h>
#include <beCore/beParameters.h>
#include <beEntitySystem/beSerializationParameters.h>
#include <beEntitySystem/beEntity.h>
#include <beEntitySystem/beController.h>
#include <lean/smart/resource_ptr.h>

#include <QtGui/QUndoStack.h>
#include "Commands/CreateEntity.h"

#include "Utility/InputProvider.h"

#include "Interaction/Math.h"
#include <beMath/bePlane.h>

#include "Utility/Strings.h"
#include "Utility/Undo.h"
#include "Utility/Checked.h"

namespace
{

/// Drag/drap entity creation interaction.
class EntityCreationInteraction : public DropInteraction
{
private:
	Editor *m_pEditor;

	SceneDocument *m_pDocument;

	const Ui::EntityBuilderWidget *m_pUI;
	QWidget *m_pWidget;

	const CreateEntityCommand *m_pCommand;

	/// Creates the entity.
	beEntitySystem::Entity* entity()
	{
		if (m_pCommand)
			return m_pCommand->entities()[0];

		QString name = m_pUI->nameLineEdit->text();

		try
		{
			// Entity
			lean::resource_ptr<beEntitySystem::Entity> pEntity = lean::new_resource<beEntitySystem::Entity>( toUtf8Range(name) );

			// Document & entity environment
			beCore::ParameterSet sceneParams( &beEntitySystem::GetSerializationParameters() );
			m_pDocument->setSerializationParameters(sceneParams);
			SetEntityParameter(sceneParams, pEntity);

			QList<ControllerBuilderWidget*> controllerWidgets = m_pUI->controllerAreaContents->findChildren<ControllerBuilderWidget*>();

			// Controllers
			Q_FOREACH (ControllerBuilderWidget *controllerWidget, controllerWidgets)
			{
				// Controller-specific parameters
				beCore::Parameters creationParams;
				controllerWidget->setParameters(creationParams, *m_pDocument);

				// Controller
				lean::resource_ptr<beEntitySystem::Controller> pController =
					controllerWidget->serializer()->Create(creationParams, sceneParams);
				pEntity->AddController(pController);
			}

			CreateEntityCommand *pCommand( new CreateEntityCommand(m_pDocument, m_pDocument->world(), pEntity) );
			m_pDocument->undoStack()->push(pCommand);
			m_pCommand = pCommand;
		}
		catch (...)
		{
			exceptionToMessageBox(
					EntityBuilderWidget::tr("Error creating entity"),
					EntityBuilderWidget::tr("An unexpected error occurred while creating entity '%1'.").arg( makeName(name) )
				);
		}

		return (m_pCommand) ? m_pCommand->entities()[0] : nullptr;
	}

public:
	/// Constructor.
	EntityCreationInteraction(SceneDocument *pDocument, const Ui::EntityBuilderWidget *pUI,
		QWidget *pWidget, Editor *pEditor)
		: m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
		m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
		m_pUI( LEAN_ASSERT_NOT_NULL(pUI) ),
		m_pWidget( LEAN_ASSERT_NOT_NULL(pWidget) ),
		m_pCommand()
	{
	}

	/// Steps the interaction.
	void step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective)
	{
		using namespace beMath::Types;

		beEntitySystem::Entity *pEntity = entity();

		if (pEntity)
		{
			fvec3 rayOrig, rayDir;
			camRayDirUnderCursor(rayOrig, rayDir, toQt(input.relativePosition()), perspective.ViewProjMat);

			fplane3 movePlane;

			if (input.keyPressed(Qt::Key_Control) || input.buttonPressed(Qt::MiddleButton))
			{
				fvec3 toEntity = pEntity->GetPosition() - rayOrig;

//				if (abs(dot(toEntity, perspective.CamRight)) > abs(dot(toEntity, perspective.CamUp)))
//					movePlane = mkplane(perspective.CamRight, m_pEntity->GetPosition());
//				else
					movePlane = mkplane(perspective.CamUp, pEntity->GetPosition());
			}
			else
				movePlane = mkplane(perspective.CamLook, pEntity->GetPosition());

			pEntity->SetPosition( rayOrig + rayDir * intersect(movePlane, rayOrig, rayDir) );
		}
	}

	/// Accepts the given drop event, if matching.
	void accept(QDropEvent &dropEvent)
	{
		// Accept, if right event & creation successful
		if (dropEvent.source() == m_pWidget && entity())
		{
			dropEvent.acceptProposedAction();

			redoIfMostRecent(*m_pDocument->undoStack(), *m_pCommand);
		}
	}
	/// Cancels the entity creation.
	void cancel()
	{
		undoIfMostRecent(*m_pDocument->undoStack(), *m_pCommand);
	}
};

/// Adds controller types to the given item widget.
void setControllerTypes(QPushButton &button, EntityBuilderWidget &widget)
{
	const beEntitySystem::ControllerSerialization &serialization = beEntitySystem::GetControllerSerialization();

	QVector<const beEntitySystem::ControllerSerializer*> serializers(serialization.GetSerializerCount());
	serialization.GetSerializers(serializers.data());

	std::auto_ptr<QMenu> menu( new QMenu(&button) );
	QSignalMapper *pMapper = new QSignalMapper(menu.get());

	Q_FOREACH (const beEntitySystem::ControllerSerializer *serializer, serializers)
	{
		QString controllerType = QString::fromUtf8(serializer->GetType().c_str());
		QAction *pAction = menu->addAction(controllerType);
		
		pMapper->setMapping(pAction, controllerType);
		checkedConnect(pAction, SIGNAL(triggered()), pMapper, SLOT(map()));
	}

	checkedConnect(pMapper, SIGNAL(mapped(const QString&)), &widget, SLOT(addController(const QString&)));
	button.setMenu(menu.release());
}

} // namespace

// Constructor.
EntityBuilderWidget::EntityBuilderWidget(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pDocument()
{
	ui.setupUi(this);

	setControllerTypes(*ui.controllerButton, *this);

	checkedConnect(ui.createButton, SIGNAL(pressed()), this, SLOT(createEntity()), Qt::QueuedConnection);

	setDocument(nullptr);
}

// Destructor.
EntityBuilderWidget::~EntityBuilderWidget()
{
}

// Sets the current document.
void EntityBuilderWidget::setDocument(AbstractDocument *pDocument)
{
	m_pDocument = qobject_cast<SceneDocument*>(pDocument);

	ui.createButton->setEnabled(m_pDocument != nullptr);
}

// Creates an entity.
void EntityBuilderWidget::createEntity()
{
	if (m_pDocument)
	{
		std::auto_ptr<EntityCreationInteraction> pInteraction(
			new EntityCreationInteraction(m_pDocument, &ui, this, m_pEditor) );

		QDrag *drag = new QDrag(this);
		drag->setPixmap( QPixmap(QString::fromUtf8(":/breezEd/icons/tree/asset")) );
		drag->setMimeData( new QMimeData() );

		m_pDocument->pushInteraction(pInteraction.get());
		drag->exec();
		m_pDocument->removeInteraction(pInteraction.get());
	}
}

// Adds a controller of the given type.
void EntityBuilderWidget::addController(const QString &type)
{
	const beEntitySystem::ControllerSerializer *pSerializer = beEntitySystem::GetControllerSerialization().GetSerializer( toUtf8Range(type) );

	if (pSerializer)
	{
		ControllerBuilderWidget *pControllerWidget = new ControllerBuilderWidget(pSerializer, m_pEditor, ui.controllerAreaContents);
		pControllerWidget->setAttribute(Qt::WA_DeleteOnClose);

		checkedConnect(pControllerWidget, SIGNAL(moveUp()), this, SLOT(moveUp()));
		checkedConnect(pControllerWidget, SIGNAL(moveDown()), this, SLOT(moveDown()));

		ui.controllerAreaLayout->addWidget(pControllerWidget);
		pControllerWidget->show();
	}
}

// Moves the given controller up.
void EntityBuilderWidget::moveUp()
{
	ControllerBuilderWidget *pWidget = qobject_cast<ControllerBuilderWidget*>(sender());
	int index = ui.controllerAreaLayout->indexOf(pWidget);

	if (index > 0)
		ui.controllerAreaLayout->insertWidget(index - 1, pWidget);
}

// Moves the given controller down.
void EntityBuilderWidget::moveDown()
{
	ControllerBuilderWidget *pWidget = qobject_cast<ControllerBuilderWidget*>(sender());
	int index = ui.controllerAreaLayout->indexOf(pWidget);

	if (index >= 0)
		ui.controllerAreaLayout->insertWidget(index + 1, pWidget);
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"

namespace
{

/// Plugin class.
struct EntityBuilderWidgetPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	EntityBuilderWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~EntityBuilderWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *pMainWindow) const
	{
		QDockWidget *pDock = new QDockWidget(pMainWindow);
		pDock->setObjectName("EntityBuilderWidget");
		pDock->setWidget( new EntityBuilderWidget(pMainWindow->editor(), pDock) );
		pDock->setWindowTitle(pDock->widget()->windowTitle());
		pDock->setWindowIcon(pDock->widget()->windowIcon());
		
		// Invisible by default
		pMainWindow->addDockWidget(Qt::LeftDockWidgetArea, pDock);
		pDock->hide();

		checkedConnect(pMainWindow->widgets().actionEntity_Builder, SIGNAL(triggered()), pDock, SLOT(show()));
		checkedConnect(pMainWindow, SIGNAL(documentChanged(AbstractDocument*)), pDock->widget(), SLOT(setDocument(AbstractDocument*)));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const EntityBuilderWidgetPlugin EntityBuilderWidgetPlugin;

} // namespace
