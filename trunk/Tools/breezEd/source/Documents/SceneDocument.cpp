#include "stdafx.h"
#include "Documents/SceneDocument.h"

#include "Interaction/DropInteraction.h"

#include <QtGui/QUndoStack>

#include "Editor.h"
#include "DeviceManager.h"

#include <beEntitySystem/beSerializationParameters.h>
#include <beScene/beSerializationParameters.h>
#include <bePhysics/beSerializationParameters.h>

#include <beScene/beShaderDrivenPipeline.h>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

// Scene document type name.
const char *const SceneDocument::DocumentTypeName = "Scene";

// Constructor.
SceneDocument::SceneDocument(const QString &type, const QString &name, const QString &file, bool bLoadFromFile, Editor *pEditor, QObject *pParent)
	: AbstractDocument(type, name, file, pEditor, pParent),
	m_pUndoStack( new QUndoStack(this) ),
	m_pWorld( nullptr ),
	m_pSimulation( lean::new_resource<beEntitySystem::Simulation>( toUtf8Range(name) ) ),
	m_pRenderer( beScene::CreateEffectDrivenRenderer(editor()->deviceManager()->graphicsDevice()) ),
	m_pRenderContext( beScene::CreateRenderContext(m_pRenderer->ImmediateContext()) ),
	m_pScene( lean::new_resource<beScene::SceneController>(m_pSimulation, m_pRenderer->Pipeline(), m_pRenderContext) ),
	m_pPhysics( lean::new_resource<bePhysics::SceneController>(m_pSimulation, editor()->deviceManager()->physicsDevice()) ),
	m_pPrimaryView(),
	m_pDropInteraction()
{
	// Keep documents in sync
	checkedConnect(m_pUndoStack, SIGNAL(cleanChanged(bool)), this, SLOT(setClean(bool)));
	checkedConnect(this, SIGNAL(documentClean()), m_pUndoStack, SLOT(setClean()));

	// TODO: read from somewhere
	beScene::LoadRenderingPipeline(*m_pRenderer->Pipeline(),
		*editor()->deviceManager()->graphicsResources()->EffectCache()->GetEffect("Pipelines/LPR/Pipeline.fx", nullptr, 0),
		*m_pRenderer->RenderableDrivers());

	m_pSimulation->Pause(true);

	m_pSimulation->AddController(m_pScene);
	m_pSimulation->AddController(m_pPhysics);
	m_pSimulation->Attach();

	if (bLoadFromFile)
	{
		beCore::ParameterSet serializationParams( getSerializationParameters() );
		m_pWorld = lean::new_resource<beEntitySystem::World>( toUtf8Range(name), toUtf8Range(file), serializationParams );
	}
	else
		m_pWorld = lean::new_resource<beEntitySystem::World>( toUtf8Range(name) ); 

	m_pWorld->Attach();
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

// Adds the given interaction.
void SceneDocument::pushInteraction(DropInteraction *pInteraction)
{
	pushInteraction( static_cast<Interaction*>(pInteraction) );
	m_pDropInteraction = pInteraction;
}

// Removes the given interaction.
void SceneDocument::removeInteraction(Interaction *pInteraction)
{
	if (pInteraction == m_pDropInteraction)
		m_pDropInteraction = nullptr;

	int idx = m_interactions.indexOf(pInteraction);
	
	if (idx != -1)
	{
		m_interactions.remove(idx);
		pInteraction->detach();
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
	beEntitySystem::World::Entities entities = m_pWorld->GetEntities();
	EntityVector selection(entities.size());
	std::copy(entities.begin(), entities.end(), selection.begin());
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
	beEntitySystem::EntitySystemParameters entityParameters(m_pSimulation);
	SetEntitySystemParameters(serializationParams, entityParameters);

	beScene::SceneParameters sceneParameters(
			editor()->deviceManager()->graphicsResources(),
			m_pRenderer,
			m_pScene, m_pScene->GetScenery()
		);
	SetSceneParameters(serializationParams, sceneParameters);

	bePhysics::PhysicsParameters physicsParameters(
			editor()->deviceManager()->physicsDevice(),
			editor()->deviceManager()->physicsResources(),
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
	QWidget* createDocumentView(AbstractDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
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
