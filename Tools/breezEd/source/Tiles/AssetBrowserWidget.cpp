#include "stdafx.h"
#include "Tiles/AssetBrowserWidget.h"

#include "Documents/SceneDocument.h"

#include "Binders/FileTreeBinder.h"

#include "Interaction/DropInteraction.h"
#include "Interaction/Math.h"
#include <beMath/bePlane.h>

#include "Commands/CreateEntity.h"

#include <beCore/beParameterSet.h>
#include <beEntitySystem/beEntities.h>
#include <beEntitySystem/beEntityGroup.h>
#include <beEntitySystem/beSerializationParameters.h>
#include <beEntitySystem/beAsset.h>

#include "Utility/IconDockStyle.h"
#include "Utility/InputProvider.h"
#include "Utility/Undo.h"
#include "Utility/Strings.h"
#include "Utility/Checked.h"

/// Drag/drap asset creation interaction.
class AssetBrowserWidget::CreationInteraction : public DropInteraction
{
private:
	Editor *m_pEditor;

	SceneDocument *m_pDocument;

	QWidget *m_pWidget;

	QString m_name;
	QString m_file;
	const CreateEntityCommand *m_pCommand;
	bool m_bPlaced;

	/// Creates the entity.
	CreateEntityCommand::EntityRange entities()
	{
		if (m_pCommand)
			return m_pCommand->entities();

		try
		{
			beEntitySystem::EntityGroup asset;

			// Load asset
			beCore::ParameterSet sceneParams = m_pDocument->getSerializationParameters();
			beEntitySystem::SetNoOverwriteParameter(sceneParams, true);

			beEntitySystem::LoadAsset(m_pDocument->world()->Entities(), asset, toUtf8(m_file), sceneParams);
			
			// Prepare entitites
			beEntitySystem::EntityGroup::EntityRange entities = asset.GetEntities();

//			for (CreateEntityCommand::EntityRange::const_iterator it = entities.begin(); it != entities.end(); ++it)
//				(*it)->SetPersistentID(beCore::PersistentIDs::InvalidID);

			CreateEntityCommand *pCommand = new CreateEntityCommand(
					m_pDocument, m_pDocument->world(),
					entities.begin(), entities.size(),
					m_name
				);
			m_pDocument->undoStack()->push(pCommand);
			m_pCommand = pCommand;
		}
		catch (...)
		{
			exceptionToMessageBox(
					tr("Error creating entity"),
					tr("An unexpected error occurred while creating entity '%1'.").arg( makeName(m_name) )
				);
		}

		return (m_pCommand) ? m_pCommand->entities() : CreateEntityCommand::EntityRange();
	}

public:
	/// Constructor.
	CreationInteraction(const QString &name, const QString &file, SceneDocument *pDocument, QWidget *pWidget, Editor *pEditor)
		: m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
		m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
		m_pWidget( LEAN_ASSERT_NOT_NULL(pWidget) ),
		m_name(name),
		m_file(file),
		m_pCommand(),
		m_bPlaced(false)
	{
	}

	/// Steps the interaction.
	void step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective)
	{
		using namespace beMath::Types;

		CreateEntityCommand::EntityRange entities = this->entities();

		if (!entities.empty())
		{
			fvec3 center;
			for (CreateEntityCommand::EntityRange::const_iterator it = entities.begin(); it != entities.end(); ++it)
				center += (*it)->GetPosition();
			center /= (float) entities.size();

			fvec3 rayOrig, rayDir;
			camRayDirUnderCursor(rayOrig, rayDir, toQt(input.relativePosition()), perspective.ViewProjMat);

			fvec3 planeBase = center;
			if (!m_bPlaced)
			{
				planeBase = perspective.CamPos + 20.0f * perspective.CamLook;
				m_bPlaced = true;
			}

			fplane3 movePlane;

			if (input.keyPressed(Qt::Key_Control) || input.buttonPressed(Qt::MiddleButton))
				movePlane = mkplane(perspective.CamUp, planeBase);
			else
				movePlane = mkplane(perspective.CamLook, planeBase);

			fvec3 newCenter = rayOrig + rayDir * intersect(movePlane, rayOrig, rayDir);
			fvec3 moveCenter = newCenter - center;

			for (CreateEntityCommand::EntityRange::const_iterator it = entities.begin(); it != entities.end(); ++it)
			{
				bees::Entity *entity = *it;
				entity->SetPosition(entity->GetPosition() + moveCenter);
				entity->NeedSync();
			}
		}
	}

	/// Accepts the given drop event, if matching.
	void accept(QDropEvent &dropEvent)
	{
		// Accept, if right event & creation successful
		if (dropEvent.source() == m_pWidget && !entities().empty())
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

// Constructor.
AssetBrowserWidget::AssetBrowserWidget(Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pDocument()
{
	ui.setupUi(this);

	QIcon assetIcon(QString::fromUtf8(":/breezEd/icons/tools/asset"));

	new FileTreeBinder("Data/Assets", QStringList("*.asset.xml"), assetIcon, ui.treeWidget, nullptr, ui.treeWidget);

	connect(ui.treeWidget, &DragTreeWidget::performDrag, this, &AssetBrowserWidget::performDrag, Qt::DirectConnection);
}

// Destructor.
AssetBrowserWidget::~AssetBrowserWidget()
{
}

// Handles asset dragging.
void AssetBrowserWidget::performDrag(QTreeWidgetItem *pItem, bool &bPerformed)
{
	if (m_pDocument && pItem)
	{
		CreationInteraction interaction(
				pItem->text(0), pItem->toolTip(0),
				m_pDocument, this, m_pEditor
			);
		
		lean::scoped_ptr<QDrag> drag( new QDrag(this) );
		drag->setPixmap( pItem->icon(0).pixmap(16) );
		drag->setMimeData( interactionMimeData(&interaction) );
		drag.detach()->exec();
		
		bPerformed = true;
	}
}

// Sets the current document.
void AssetBrowserWidget::setDocument(AbstractDocument *pDocument)
{
	m_pDocument = qobject_cast<SceneDocument*>(pDocument);
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"
#include "Docking/DockContainer.h"

namespace
{

/// Plugin class.
struct AssetBrowserWidgetPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	AssetBrowserWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~AssetBrowserWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *mainWindow) const
	{
/*		lean::scoped_ptr<QDockWidget> dock( new QDockWidget(mainWindow) );
		dock->setObjectName("AssetBrowserWidget");
		dock->setStyle( new IconDockStyle(dock, dock->style()) );

		dock->setWidget( new AssetBrowserWidget(mainWindow->editor(), dock) );
		dock->setWindowTitle(dock->widget()->windowTitle());
		dock->setWindowIcon(dock->widget()->windowIcon());
*/		
		lean::scoped_ptr<AssetBrowserWidget> widget( new AssetBrowserWidget(mainWindow->editor()) );
		lean::scoped_ptr<DockWidget> dock( DockWidget::wrap(widget) );

		// Invisible by default
		dock->hide();
		mainWindow->dock()->addDock(dock, DockPlacement::Emplace, DockOrientation::Horizontal, DockSide::Before);
//		pMainWindow->addDockWidget(Qt::LeftDockWidgetArea, dock);

		checkedConnect(mainWindow->widgets().actionAsset_Browser, SIGNAL(triggered()), dock, SLOT(showAndRaise()));
		checkedConnect(mainWindow, SIGNAL(documentChanged(AbstractDocument*)), widget, SLOT(setDocument(AbstractDocument*)));
		
		widget.detach();
		dock.detach();
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const AssetBrowserWidgetPlugin AssetBrowserWidgetPlugin;

} // namespace
