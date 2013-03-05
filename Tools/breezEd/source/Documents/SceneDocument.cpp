#include "stdafx.h"
#include "Documents/SceneDocument.h"

#include "Interaction/DropInteraction.h"

#include <QtWidgets/QUndoStack>

#include "Editor.h"
#include "DeviceManager.h"

#include <beEntitySystem/beSerializationParameters.h>
#include <beScene/beSerializationParameters.h>
#include <bePhysics/beSerializationParameters.h>

#include <beScene/beShaderDrivenPipeline.h>

#include <beEntitySystem/beEntities.h>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

// Scene document type name.
const char *const SceneDocument::DocumentTypeName = "Scene";

// Constructor.
SceneDocument::SceneDocument(const QString &type, const QString &name, const QString &file, bool bLoadFromFile, Editor *pEditor, QObject *pParent)
	: AbstractDocument(type, name, file, pEditor, pParent),
	m_pUndoStack( new QUndoStack(this) ),
	m_pGraphicsResources( beScene::CreateResourceManager(editor()->deviceManager()->graphicsDevice(), "EffectCache", "Effects", "Textures", "Materials", "Meshes") ),
	m_pPhysicsResources( bePhysics::CreateResourceManager(editor()->deviceManager()->physicsDevice(), "PhysicsMaterials", "PhysicsShapes", m_pGraphicsResources->Monitor()) ),
	m_pRenderer( beScene::CreateEffectDrivenRenderer(editor()->deviceManager()->graphicsDevice(), m_pGraphicsResources->Monitor()) ),
	m_pRenderContext( beScene::CreateRenderContext(*m_pRenderer->ImmediateContext()) ),
	m_pPrimaryView()
{
	
	// TODO: read from somewhere
	beScene::LoadRenderingPipeline(*m_pRenderer->Pipeline(),
		m_pGraphicsResources->EffectCache()->GetByFile("Pipelines/LPR/Pipeline.fx", nullptr, 0),
		*m_pRenderer->RenderableDrivers());

	{
		lean::scoped_ptr<bees::WorldControllers> controllers = new_scoped bees::WorldControllers();
	
		m_pScene = new besc::RenderingController(m_pRenderer->Pipeline(), m_pRenderContext);
		controllers->AddControllerKeep(m_pScene);

		m_pPhysics = new bepx::SceneController(editor()->deviceManager()->physicsDevice());
		controllers->AddControllerKeep(m_pPhysics);

		if (bLoadFromFile)
			m_pWorld = new_resource bees::World( toUtf8Range(name), toUtf8Range(file), getSerializationParameters(), controllers.move_ptr() );
		else
			m_pWorld = new_resource bees::World( toUtf8Range(name), controllers.move_ptr() );
	}

	m_pSimulation = new_resource bees::Simulation( toUtf8Range(name) );
	m_pSimulation->Pause(true);
	Attach(m_pWorld, m_pSimulation);

	// Keep documents in sync
	checkedConnect(m_pUndoStack, SIGNAL(cleanChanged(bool)), this, SLOT(setClean(bool)));
	checkedConnect(this, SIGNAL(documentClean()), m_pUndoStack, SLOT(setClean()));
}

// Destructor.
SceneDocument::~SceneDocument()
{
	releaseReferences();
}

// Releases all references to foreign resources.
void SceneDocument::releaseReferences()
{
	m_pUndoStack->clear();
	clearSelection();
}

// Adds the given interaction.
void SceneDocument::pushInteraction(Interaction *pInteraction)
{
	removeInteraction(pInteraction);
	m_interactions.push_back(pInteraction);
	pInteraction->attach();
}

// Removes the given interaction.
void SceneDocument::removeInteraction(Interaction *pInteraction)
{
	int idx = m_interactions.indexOf(pInteraction);
	
	if (idx != -1)
	{
		m_interactions.remove(idx);
		pInteraction->detach();
	}
}

// Commits changes.
void SceneDocument::commit()
{
	for (bool bRetry = true; bRetry; )
		try
		{
			// TODO
//			Q_EMIT preCommit();

			m_pGraphicsResources->Commit();
			m_pRenderer->Commit();
			m_pPhysicsResources->Commit();
			m_pWorld->Commit();

			Q_EMIT postCommit();
			bRetry = m_pGraphicsResources->Monitor->ChangesPending();
		}
		catch (...)
		{
			exceptionToMessageBox("Commit failed", "An unexpected error occurred while trying to react to changes, try to fix then retry.");
		}
}

// Clears the selection.
void SceneDocument::clearSelection()
{
	setSelection( EntityVector() );
}

// Select everything.
void SceneDocument::selectAll()
{
	beEntitySystem::Entities::Range entities = m_pWorld->Entities()->GetEntities();
	EntityVector selection(Size(entities));
	std::copy(entities.Begin, entities.End, selection.begin());
	setSelection(selection);
}

// Sets the selection.
void SceneDocument::setSelection(beEntitySystem::Entity *pEntity)
{
	if (pEntity)
		setSelection( EntityVector(1, pEntity) );
	else
		clearSelection();
}

// Sets the selection.
void SceneDocument::setSelection(const EntityVector &selection)
{
	m_selection = selection;
	Q_EMIT selectionChanged(this);
}

// Gets all available serialization parameters.
beCore::ParameterSet SceneDocument::getSerializationParameters()
{
	beCore::ParameterSet serializationParams( &beEntitySystem::GetSerializationParameters() );
	setSerializationParameters(serializationParams);
	return serializationParams;
}

// Sets all available serialization parameters.
void SceneDocument::setSerializationParameters(beCore::ParameterSet &serializationParams)
{
	beEntitySystem::EntitySystemParameters entityParameters(m_pWorld);
	SetEntitySystemParameters(serializationParams, entityParameters);

	beScene::SceneParameters sceneParameters(
			m_pGraphicsResources,
			m_pRenderer, m_pScene
		);
	SetSceneParameters(serializationParams, sceneParameters);

	bePhysics::PhysicsParameters physicsParameters(
			editor()->deviceManager()->physicsDevice(),
			m_pPhysicsResources,
			m_pPhysics, m_pPhysics->GetScene()
		);
	SetPhysicsParameters(serializationParams, physicsParameters);
}

// Saves the document using the given filename.
bool SceneDocument::saveAs(const QString &file)
{
	try
	{
		QDir directory = QFileInfo(file).absoluteDir();

		if (!directory.exists())
			QFileInfo(directory.path()).dir().mkdir(directory.dirName());

		m_pWorld->Serialize( toUtf8Range(file) );
	}
	catch (...)
	{
		return false;
	}

	// Clean state
	setFile(file);
	setChanged(false);

	// Status update
	editor()->showMessage( AbstractDocument::tr("Saved document '%1' to '%2'.").arg(name()).arg(file) );

	return true;
}

// Sets the document name.
void SceneDocument::setName(const QString &name)
{
	m_pWorld->SetName( toUtf8Range(name) );
	m_pSimulation->SetName(m_pWorld->GetName());
	setChanged(true);

	AbstractDocument::setName(name);
}

// Device manager.
DeviceManager* SceneDocument::deviceManager() const
{
	return editor()->deviceManager();
}

#include "Documents/AbstractDocumentFactory.h"
#include "Views/SceneView.h"
#include "Modes/Mode.h"
#include "Modes/DocumentModeState.h"
#include "Modes/SceneModeState.h"
#include "Modes/EntityModeState.h"
#include "Modes/ParallelMode.h"

namespace
{

/// Scene document factory class.
class SceneDocumentFactory : public AbstractDocumentFactory
{
public:
	/// Constructor.
	SceneDocumentFactory(QObject *pParent = nullptr)
		: AbstractDocumentFactory(pParent) { }
	/// Destructor.
	virtual ~SceneDocumentFactory() { }

	/// Creates a document.
	AbstractDocument* createDocument(const QString &name, const QString &file, Editor *pEditor, QObject *pParent)
	{
		return new SceneDocument(SceneDocument::DocumentTypeName, name, file, false, pEditor, pParent);
	}

	/// Builds a file path from the given name & directory.
	QString fileFromDir(const QString &name, const QString &path) const
	{
		QFileInfo fileInfo( QFileInfo( path, QFileInfo(name).baseName() ).filePath(), name );
		QString absolutePath = fileInfo.absoluteFilePath();

		if (fileInfo.completeSuffix() != ".scene.xml")
			absolutePath += ".scene.xml";

		return absolutePath;
	}

	/// Opens a document.
	AbstractDocument* openDocument(const QString &file, Editor *pEditor, QObject *pParent)
	{
		// TODO: Load from file
		QString name = QFileInfo(file).fileName();

		return new SceneDocument(SceneDocument::DocumentTypeName, name, file, true, pEditor, pParent);
	}
	
	/// Checks if the given file may be opened using this document factory.
	FileMatch::T matchFile(const QString &file) const
	{
		return (QFileInfo(file).completeSuffix() == "scene.xml") ? FileMatch::Match : FileMatch::Mismatch;
	}

	/// Creates a default mdi document view.
	QWidget* createDocumentView(AbstractDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	{
		DocumentModeState *pDocumentModeState = pDocumentMode->findChild<DocumentModeState*>();
		Mode *pViewModes = LEAN_ASSERT_NOT_NULL(pDocumentModeState)->viewModes();
		return new SceneView( qobject_cast<SceneDocument*>(pDocument), LEAN_ASSERT_NOT_NULL(pViewModes), pEditor, pParent, flags );
	}

	/// Creates a document mode.
	Mode* createDocumentMode(AbstractDocument *pDocument, Editor *pEditor, Mode *pParent)
	{
		std::auto_ptr<Mode> pMode( new Mode(pParent) );

		DocumentModeState *pDocumentModeState = new DocumentModeState(pDocument, pEditor, pMode.get());
		pMode->addState( pDocumentModeState );

		pMode->addState( new SceneModeState(qobject_cast<SceneDocument*>(pDocument), pEditor, pMode.get()) );
		
		Mode *entityMode = new Mode(pMode.get());
		pMode->setDefaultChildMode(entityMode);
		entityMode->addState( new EntityModeState(qobject_cast<SceneDocument*>(pDocument), pEditor, entityMode) );

		pDocumentModeState->setViewModes( new Mode(pDocumentModeState) );
		pMode->addState( new ParallelMode(pDocumentModeState->viewModes(), pMode.get()) );
		
		return pMode.release();
	}
};

} // namespace

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Editor.h"
#include "Documents/DocumentManager.h"

namespace
{

/// Plugin class.
struct SceneDocumentPlugin : public AbstractPlugin<Editor*>
{
	mutable SceneDocumentFactory factory;

	/// Constructor.
	SceneDocumentPlugin()
	{
		editorPlugins().addPlugin(this);
	}

	/// Destructor.
	~SceneDocumentPlugin()
	{
		editorPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(Editor *pEditor) const
	{
		pEditor->documentManager()->addDocumentType(
				DocumentType(
					SceneDocument::DocumentTypeName,
					SceneDocument::tr("Generic scene."),
					&factory,
					QIcon(":/breezEd/images/documentType/scene")
				)
			);
	}
	/// Finalizes the plugin.
	void finalize(Editor *pEditor) const
	{
		pEditor->documentManager()->removeDocumentType(SceneDocument::DocumentTypeName);
	}
};

const SceneDocumentPlugin SceneDocumentPlugin;

} // namespace
