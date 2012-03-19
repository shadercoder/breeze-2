#include "stdafx.h"
#include "Tiles/AssetBrowserWidget.h"

#include "Documents/SceneDocument.h"

#include "Binders/FileTreeBinder.h"

#include "Interaction/DropInteraction.h"
#include "Interaction/Math.h"
#include <beMath/bePlane.h>

#include "Commands/CreateEntity.h"

#include <beCore/beParameterSet.h>
#include <beEntitySystem/beEntity.h>
#include <beEntitySystem/beEntityGroup.h>
#include <beEntitySystem/beSerializationParameters.h>
#include <beEntitySystem/beAsset.h>

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

			beEntitySystem::LoadAsset(asset, toUtf8(m_file), sceneParams);
			
			// Prepare entitites
			beEntitySystem::EntityGroup::EntityRange entities = asset.GetEntities();

			for (CreateEntityCommand::EntityRange::const_iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->SetPersistentID(beCore::PersistentIDs::InvalidID);

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
		m_pCommand()
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

			fplane3 movePlane;

			if (input.keyPressed(Qt::Key_Control) || input.buttonPressed(Qt::MiddleButton))
				movePlane = mkplane(perspective.CamUp, center);
			else
				movePlane = mkplane(perspective.CamLook, center);

			fvec3 newCenter = rayOrig + rayDir * intersect(movePlane, rayOrig, rayDir);
			fvec3 moveCenter = newCenter - center;

			for (CreateEntityCommand::EntityRange::const_iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->SetPosition( (*it)->GetPosition() + moveCenter );
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
AssetBrowserWidget::AssetBrowserWidget(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pDocument()
{
	ui.setupUi(this);

	QIcon assetIcon(QString::fromUtf8(":/breezEd/icons/tools/asset"));

	new FileTreeBinder("Data/Assets", QStringList("*.asset.xml"), assetIcon, ui.treeWidget, nullptr, ui.treeWidget);

	checkedConnect(ui.treeWidget, SIGNAL(dragStarted(QTreeWidgetItem*)), this, SLOT(dragStarted(QTreeWidgetItem*)));
	checkedConnect(ui.treeWidget, SIGNAL(dragFinished(QTreeWidgetItem*)), this, SLOT(dragFinished(QTreeWidgetItem*)));
}

// Destructor.
AssetBrowserWidget::~AssetBrowserWidget()
{
}

// Handles asset dragging.
void AssetBrowserWidget::dragStarted(QTreeWidgetItem *pItem)
{
	if (m_pDocument && pItem)
	{
		LEAN_ASSERT(m_pInteraction == nullptr);

		m_pInteraction = new CreationInteraction(
			pItem->text(0), pItem->toolTip(0),
			m_pDocument, this, m_pEditor);

		m_pDocument->pushInteraction(m_pInteraction);
	}
}

// Handles asset dragging.
void AssetBrowserWidget::dragFinished(QTreeWidgetItem *pItem)
{
	if (m_pInteraction)
	{
		if (m_pDocument)
			m_pDocument->removeInteraction(m_pInteraction);

		m_pInteraction = nullptr;
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
	void initialize(MainWindow *pMainWindow) const
	{
		QDockWidget *pDock = new QDockWidget(pMainWindow);
		pDock->setObjectName("AssetBrowserWidget");
		pDock->setWidget( new AssetBrowserWidget(pMainWindow->editor(), pDock) );
		pDock->setWindowTitle(pDock->widget()->windowTitle());
		pDock->setWindowIcon(pDock->widget()->windowIcon());
		
		// Invisible by default
		pMainWindow->addDockWidget(Qt::LeftDockWidgetArea, pDock);
		pDock->hide();

		checkedConnect(pMainWindow->widgets().actionAsset_Browser, SIGNAL(triggered()), pDock, SLOT(show()));
		checkedConnect(pMainWindow, SIGNAL(documentChanged(AbstractDocument*)), pDock->widget(), SLOT(setDocument(AbstractDocument*)));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const AssetBrowserWidgetPlugin AssetBrowserWidgetPlugin;

} // namespace
