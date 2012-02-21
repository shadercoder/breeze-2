#include "stdafx.h"
#include "Binders/EntityPropertyBinder.h"
#include "Binders/GenericPropertyBinder.h"

#include <QtGui/QTreeView>

#include <beEntitySystem/beController.h>

#include "Utility/Strings.h"

// Constructor.
EntityPropertyBinder::EntityPropertyBinder(beEntitySystem::Entity *pEntity,
										   QTreeView *pTree, QStandardItem *pParentItem,
										   QObject *pParent)
	: QObject(pParent),
	m_pEntity( LEAN_ASSERT_NOT_NULL(pEntity) )
{
	// Create new empty model, if none to be extended
	if (!pParentItem)
	{
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
	
	new GenericPropertyBinder(m_pEntity, pTree, pEntityItem, this);
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

		new GenericPropertyBinder(pController, pTree, pControllerItem, this);
		pTree->expand( pControllerItem->index() );
	}
}

// Destructor.
EntityPropertyBinder::~EntityPropertyBinder()
{
}
