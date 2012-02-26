#include "stdafx.h"
#include "Binders/EntityPropertyBinder.h"
#include "Binders/GenericPropertyBinder.h"

#include <QtGui/QTreeView>

#include <beEntitySystem/beController.h>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

// Constructor.
EntityPropertyBinder::EntityPropertyBinder(beEntitySystem::Entity *pEntity,
										   SceneDocument *pDocument,
										   QTreeView *pTree, QStandardItem *pParentItem,
										   QObject *pParent)
	: QObject(pParent),
	m_pEntity( LEAN_ASSERT_NOT_NULL(pEntity) )
{
	// Create new empty model, if none to be extended
	if (!pParentItem)
	{
		GenericPropertyBinder::setupTree(*pTree);

		// WARNING: Don't attach to binder, binder might be attached to model
		QStandardItemModel *pModel = new QStandardItemModel(pTree);
		GenericPropertyBinder::setupModel(*pModel);
		pTree->setModel(pModel);

		pParentItem = pModel->invisibleRootItem();
	}

	QStandardItem *pEntityItem = new QStandardItem( toQt(pEntity->GetType()) );
	pEntityItem->setFlags(Qt::ItemIsEnabled);
	pParentItem->appendRow(pEntityItem);
	GenericPropertyBinder::fillRow(*pEntityItem);
	
	GenericPropertyBinder *pEntityBinder = new GenericPropertyBinder(m_pEntity, m_pEntity, pDocument, pTree, pEntityItem, this);
	checkedConnect(this, SIGNAL(propagateUpdateProperties()), pEntityBinder, SLOT(updateProperties()));
	pTree->expand( pEntityItem->index() );

	beEntitySystem::Entity::Controllers controllers = m_pEntity->GetControllers();

	for (beEntitySystem::Entity::Controllers::const_iterator it = controllers.begin();
		it != controllers.end(); ++it)
	{
		beEntitySystem::Controller *pController = *it;

		QStandardItem *pControllerItem = new QStandardItem( toQt(pController->GetType()) );
		pControllerItem->setFlags(Qt::ItemIsEnabled);
		pParentItem->appendRow( pControllerItem );
		GenericPropertyBinder::fillRow(*pControllerItem);

		GenericPropertyBinder *pBinder = new GenericPropertyBinder(pController, pController, pDocument, pTree, pControllerItem, this);
		checkedConnect(this, SIGNAL(propagateUpdateProperties()), pBinder, SLOT(updateProperties()));
		pTree->expand( pControllerItem->index() );
	}
}

// Destructor.
EntityPropertyBinder::~EntityPropertyBinder()
{
}

// Check for property changes.
void EntityPropertyBinder::updateProperties()
{
	Q_EMIT propagateUpdateProperties();
}
