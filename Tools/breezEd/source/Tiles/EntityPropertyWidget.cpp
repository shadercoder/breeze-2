#include "stdafx.h"
#include "Tiles/EntityPropertyWidget.h"

#include "Documents/SceneDocument.h"

#include "Binders/EntityPropertyBinder.h"
#include <QtGui/QStandardItemModel>

#include <QtCore/QTimer>

#include <lean/smart/scoped_ptr.h>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

/// Updates the widget from the given selection.
void updateFromSelection(Ui::EntityPropertyWidget &widget, const EntityPropertyWidget::entity_vector &selection, SceneDocument *pDocument,
	EntityPropertyWidget &listener, QTimer &timer)
{
	QString selectionText = (selection.empty())
		? EntityPropertyWidget::tr("<none>")
		: makeName(toQt(selection.front()->GetName()));

	for (int i = 1; i < selection.size(); ++i)
		selectionText += ", " + makeName(toQt(selection[i]->GetName()));

	widget.selectionLabel->setText(selectionText);

	// Delete old model on exit
	lean::scoped_ptr<QAbstractItemModel> pOldModel( widget.propertyView->model() );

	if  (!selection.empty())
	{
		EntityPropertyBinder *pBinder = new EntityPropertyBinder(selection[0], pDocument, widget.propertyView, nullptr);
		checkedConnect(&timer, SIGNAL(timeout()), pBinder, SLOT(updateProperties()));
		checkedConnect(pBinder, SIGNAL(propertiesChanged()), &listener, SLOT(propertiesChanged()));
		pBinder->setParent(widget.propertyView->model());
	}
	else if (pOldModel)
		widget.propertyView->setModel(nullptr);
}

} // namespace

// Constructor.
EntityPropertyWidget::EntityPropertyWidget(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pTimer( new QTimer(this) ),
	m_pDocument()
{
	ui.setupUi(this);
	m_pTimer->setInterval(150);
	m_pTimer->setSingleShot(true);
	updateFromSelection(ui, m_selection, m_pDocument, *this, *m_pTimer);
}

// Destructor.
EntityPropertyWidget::~EntityPropertyWidget()
{
}

// Properties have changed.
void EntityPropertyWidget::propertiesChanged()
{
	if (!m_pTimer->isActive())
		m_pTimer->start();
}

// Sets the active selection of the given document.
void EntityPropertyWidget::setActiveSelection(SceneDocument *pDocument)
{
	if (pDocument)
	{
		const SceneDocument::EntityVector &selection = pDocument->selection();

		if (selection != m_selection)
		{
			m_selection = pDocument->selection();
			updateFromSelection(ui, m_selection, pDocument, *this, *m_pTimer);
		}
	}
	else if (!m_selection.empty())
	{
		m_selection.clear();
		updateFromSelection(ui, m_selection, nullptr, *this, *m_pTimer);
	}
}

// Sets the undo stack of the given document.
void EntityPropertyWidget::setDocument(AbstractDocument *pDocument)
{
	if (m_pDocument)
		disconnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(setActiveSelection(SceneDocument*)));

	m_pDocument = qobject_cast<SceneDocument*>(pDocument);

	if (m_pDocument)
		checkedConnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(setActiveSelection(SceneDocument*)));

	setActiveSelection(m_pDocument);
}

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"

namespace
{

/// Plugin class.
struct EntityPropertyWidgetPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	EntityPropertyWidgetPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~EntityPropertyWidgetPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *pMainWindow) const
	{
		QDockWidget *pDock = new QDockWidget(pMainWindow);
		pDock->setObjectName("EntityPropertyWidget");
		pDock->setWidget( new EntityPropertyWidget(pMainWindow->editor(), pDock) );
		pDock->setWindowTitle(pDock->widget()->windowTitle());
		pDock->setWindowIcon(pDock->widget()->windowIcon());
		
		// Invisible by default
		pMainWindow->addDockWidget(Qt::RightDockWidgetArea, pDock);
		pDock->hide();

		checkedConnect(pMainWindow->widgets().actionEntity_Properties, SIGNAL(triggered()), pDock, SLOT(show()));
		checkedConnect(pMainWindow, SIGNAL(documentChanged(AbstractDocument*)), pDock->widget(), SLOT(setDocument(AbstractDocument*)));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const EntityPropertyWidgetPlugin EntityPropertyWidgetPlugin;

} // namespace
