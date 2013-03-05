#include "stdafx.h"
#include "Binders/ComponentListBinder.h"

#include <QtWidgets/QTreeView>
#include <QtGui/QStandardItemModel>

#include <beCore/beComponentReflector.h>

#include "Documents/SceneDocument.h"

#include "Utility/Strings.h"
#include "Utility/Checked.h"

#include <lean/smart/scoped_ptr.h>

namespace
{

/// Adds components to the given parent item.
QStandardItem* setComponents(const bec::ComponentReflector *reflector, const bec::ParameterSet &parameters,
							   const QIcon &componentIcon, QStandardItem *parentItem, QTreeView *tree)
{
	// Save selection
	QString currentName;
	{
		QModelIndex item = tree->currentIndex();
		
		if (item.isValid())
		{
			item = item.sibling(item.row(), 0);
			currentName = tree->model()->data(item, Qt::DisplayRole).toString();
		}
	}

	// Clear
	parentItem->removeRows(0, parentItem->rowCount());

	// Reflect components
	bec::ComponentInfoVector componentInfo = reflector->GetComponentInfo(parameters);
	
	for (const bec::ComponentInfo *info = &componentInfo[0], *infoEnd = &componentInfo[componentInfo.size()]; info < infoEnd; ++info)
	{
		QString name = toQt(info->Name);
		QString file = toQt(info->File);

		lean::scoped_ptr<QStandardItem> nameItem( new QStandardItem() );
		nameItem->setText(name);
		nameItem->setIcon(componentIcon);
		nameItem->setFlags(Qt::ItemIsEnabled| Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);
		nameItem->setData(name, Qt::UserRole);

		lean::scoped_ptr<QStandardItem> fileItem( new QStandardItem() );
		fileItem->setText(QFileInfo(file).fileName());
		fileItem->setToolTip(file);
		fileItem->setFlags(Qt::ItemIsEnabled| Qt::ItemIsSelectable);

		lean::scoped_ptr<QStandardItem> notesItem( new QStandardItem() );
		notesItem->setText(toQt(info->Notes));
		notesItem->setFlags(Qt::ItemIsEnabled| Qt::ItemIsSelectable);
		
		parentItem->appendRow( QList<QStandardItem*>() << nameItem << fileItem << notesItem);

		// Restore selection
		if (name == currentName)
			tree->setCurrentIndex(nameItem->index());

		nameItem.detach();
		fileItem.detach();
		notesItem.detach();
	}

	return parentItem;
}

} // namespace

// Constructor.
ComponentListBinder::ComponentListBinder(const beCore::ComponentTypeDesc *type, SceneDocument *document, const QIcon &icon,
										 QTreeView *tree, QStandardItem *pParentItem, QObject *pParent)
	: QObject(pParent),
	m_tree( LEAN_ASSERT_NOT_NULL(tree) ),
	m_componentIcon( icon ),
	m_type( LEAN_ASSERT_NOT_NULL(type) ),
	m_document( LEAN_ASSERT_NOT_NULL(document) )
{
	// Create new empty model, if none to be extended
	if (!pParentItem)
		pParentItem = setupTree(*m_tree)->invisibleRootItem();
	m_parentItem = pParentItem;

	updateTree();

	connect(m_tree->selectionModel(), &QItemSelectionModel::currentChanged, this, &ComponentListBinder::updateSelection);
	connect(m_tree->model(), &QAbstractItemModel::dataChanged, this, &ComponentListBinder::updateName);

	// Asynchronous
	connect(this, SIGNAL(treeChanged()), this, SLOT(updateTree()), Qt::QueuedConnection);
}

// Destructor.
ComponentListBinder::~ComponentListBinder()
{
}

// Sets the given tree view up for property editing.
QStandardItemModel* ComponentListBinder::setupTree(QTreeView &view)
{
	lean::scoped_ptr<QStandardItemModel> model( new QStandardItemModel(&view) );
	setupModel(*model);
	lean::scoped_ptr<QAbstractItemModel> oldModel( view.model() );
	view.setModel(model);
	return model.detach();
}

// Sets the given model up for component listing.
void ComponentListBinder::setupModel(QStandardItemModel &model)
{
	model.setHorizontalHeaderLabels( QStringList()
			<< ComponentListBinder::tr("Name")
			<< ComponentListBinder::tr("File")
			<< ComponentListBinder::tr("Notes")
		);
}

// Selects the given name.
void ComponentListBinder::selectName(const QString &componentName)
{
	for (int i = 0, count = m_parentItem->rowCount(); i < count; ++i)
	{
		QStandardItem *child = m_parentItem->child(i, 0);

		if (child->text() == componentName)
		{
			QModelIndex index = child->index();
			m_tree->setCurrentIndex(index);
			m_tree->scrollTo(index);
			break;
		}
	}
}

// Moves the cursor to the current item.
void ComponentListBinder::moveToCurrent()
{
	QModelIndex index = m_tree->currentIndex();

	if (index.isValid())
	{
		m_tree->scrollTo(index);
		QCursor::setPos( m_tree->viewport()->mapToGlobal( m_tree->visualRect(index).center() ) );
	}
}

// Gets the name of the component currently selected.
QString ComponentListBinder::selectedName() const
{
	QString currentName;

	QModelIndex currentIndex = m_tree->currentIndex();
	if (currentIndex.isValid())
	{
		currentIndex = currentIndex.sibling(currentIndex.row(), 0);
		currentName = m_tree->model()->data(currentIndex, Qt::DisplayRole).toString();
	}

	return currentName;
}

// The selection has changed.
void ComponentListBinder::updateSelection()
{
	Q_EMIT selectionChanged(selectedName());
}

// The name has changed.
void ComponentListBinder::updateName(const QModelIndex &start, const QModelIndex &end)
{
	if (start.column() != 0)
		return;

	for (int i = start.row(); i <= end.row(); ++i)
	{
		QModelIndex modelIndex = start.sibling(i, 0);
		QString newName = m_tree->model()->data(modelIndex, Qt::DisplayRole).toString();
		QString oldName = m_tree->model()->data(modelIndex, Qt::UserRole).toString();

		if (newName != oldName)
		{
			bool bSuccess = false;
			Q_EMIT nameChanged(oldName, newName, bSuccess);
			if (bSuccess)
				m_tree->model()->setData(modelIndex, newName, Qt::UserRole);
			else
				m_tree->model()->setData(modelIndex, oldName, Qt::DisplayRole);
		}
	}
}

// Updates the file tree.
void ComponentListBinder::updateTree()
{
	if (m_type->Reflector)
		setComponents(m_type->Reflector, m_document->getSerializationParameters(), m_componentIcon, m_parentItem, m_tree);
}
