#ifndef COLLECTIONLISTWIDGET_H
#define COLLECTIONLISTWIDGET_H

#include "breezEd.h"

#include <QtGui/QStandardItemModel>
#include <QtWidgets/QListView>
#include <QtWidgets/QShortcut>
#include "ItemDelegateEx.h"

#include <lean/smart/scoped_ptr.h>

/// Tree widget enhanced by drag signals.
class CollectionListWidget : public QListView
{
	Q_OBJECT

private:
	ItemDelegateEx *m_delegate;
	QStandardItem *m_addItem;

	/// Gets the next new item.
	QStandardItem* nextItem()
	{
		QStandardItem *newItem = m_addItem;

		m_addItem = new QStandardItem();
		m_addItem->setText(tr("<add>"));
		m_addItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		model()->appendRow(m_addItem);

		if (newItem)
		{
			newItem->setText("");
			newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
		}
		return newItem;
	}

private Q_SLOTS:
	/// Creates a new item.
	void startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled)
	{
		if (index == m_addItem->index())
			addItem();
	}

public:
	/// Constructor.
	CollectionListWidget(QWidget *pParent = nullptr)
		: QListView(pParent),
		m_delegate(),
		m_addItem()
	{
		setItemDelegate( itemDelegate() );
		setModel( new QStandardItemModel(this) );
		nextItem();

		connect(
				new QShortcut(Qt::Key_Delete, this, nullptr, nullptr, Qt::WidgetShortcut), &QShortcut::activated,
				this, &CollectionListWidget::deleteSelected
			);
		
		this->setDragDropMode(QListView::InternalMove);
		this->setSelectionMode(QListView::ExtendedSelection);
		this->setSelectionBehavior(QListView::SelectRows);
	}

	/// Adds the given item.
	QStandardItem* addItem()
	{
		// Add next placeholder item & notify
		QStandardItem *item = nextItem();
		Q_EMIT itemAdded(item);
		return item;
	}
	/// Removes the given item.
	void removeItem(QStandardItem *itemToDelete)
	{
		QStandardItemModel *model = this->model();
		int row = itemToDelete->row(), column = itemToDelete->column();
		lean::scoped_ptr<QStandardItem> item( model->takeItem(row, column) );
		model->removeRow(row);
		Q_EMIT itemDeleted(item);
	}

	/// Sets the item delegate.
	void setItemDelegate(QAbstractItemDelegate *delegate)
	{
		if (delegate != m_delegate)
		{
			ItemDelegateEx *wrappedDelegate = new ItemDelegateEx(delegate, this);
			this->QListView::setItemDelegate(wrappedDelegate);
			lean::scoped_ptr<ItemDelegateEx> oldDelegate(m_delegate);
			
			m_delegate = wrappedDelegate;
			connect(m_delegate, &ItemDelegateEx::startEditing, this, &CollectionListWidget::startEditing);
		}
	}
	/// Gets the item delegate.
	ItemDelegateEx* itemDelegateEx() const { return m_delegate; }
	/// Gets the item delegate.
	QAbstractItemDelegate* itemDelegate() const
	{
		return (m_delegate) ? m_delegate->wrapped() : this->QListView::itemDelegate();
	}
	using QListView::itemDelegate;
	
	/// Gets a list of current items.
	QList<QStandardItem*> items() const
	{
		QList<QStandardItem*> list;

		for (int i = 0; i < model()->rowCount(); ++i)
		{
			QStandardItem *item = model()->item(i);

			if (item != m_addItem)
				list.push_back(item);
		}

		return list;
	}

	/// Gets a list of current values.
	QStringList values() const
	{
		QStringList list;

		for (int i = 0; i < model()->rowCount(); ++i)
		{
			QStandardItem *item = model()->item(i);

			if (item != m_addItem)
				list.push_back(item->text());
		}

		return list;
	}

	/// Gets the model.
	QStandardItemModel* model() const { return qobject_cast<QStandardItemModel*>( this->QListView::model() ); }

public Q_SLOTS:
	/// Deletes the selected item(s).
	void deleteSelected()
	{
		for (QModelIndexList idcs; !(idcs = this->selectedIndexes()).empty(); )
			removeItem( model()->itemFromIndex(idcs.back()) );
	}

Q_SIGNALS:
	/// The given item was added.
	void itemAdded(QStandardItem *item);
	/// The given item was deleted.
	void itemDeleted(QStandardItem *item);
};

#endif