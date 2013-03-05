#include "stdafx.h"
#include "Widgets/FileCollectionWidget.h"

#include "Utility/CollectionListWidget.h"
#include "Widgets/BrowseWidget.h"

#include "Widgets/ComponentPickerFactory.h"
#include "Plugins/FactoryManager.h"

#include "Utility/Checked.h"

#include "Editor.h"

#include <lean/logging/errors.h>

FileCollectionWidget::FileCollectionWidget(Editor *editor, QWidget *pParent)
	: CollectionListWidget(pParent),
	m_editor(editor)
{
	connect(itemDelegateEx(), &ItemDelegateEx::startEditing, this, &FileCollectionWidget::startEditing, Qt::DirectConnection);
	connect(itemDelegateEx(), &ItemDelegateEx::updateEditor, this, &FileCollectionWidget::updateEditor, Qt::DirectConnection);
	connect(itemDelegateEx(), &ItemDelegateEx::updateData, this, &FileCollectionWidget::updateData, Qt::DirectConnection);
	connect(itemDelegateEx(), &ItemDelegateEx::editorGeometryChanged, this, &FileCollectionWidget::updateEditorGeometry, Qt::DirectConnection);
}

// Destructor.
FileCollectionWidget::~FileCollectionWidget()
{
}

// Editing has started.
void FileCollectionWidget::startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled)
{
	QStandardItem *item = this->model()->itemFromIndex(index);

	lean::scoped_ptr<BrowseWidget> selector( new BrowseWidget(parent) );
	connect(selector, &BrowseWidget::browse, this, &FileCollectionWidget::browse);
	selector->setPath(item->text());

	pEditor = selector.detach();
	bHandled = true;
}

// Data currently being edited has changed.
void FileCollectionWidget::updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled)
{
	bHandled = true;
}

// Editor data is requested.
void FileCollectionWidget::updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled)
{
	QStandardItem *item = this->model()->itemFromIndex(index);
	BrowseWidget *selector = LEAN_ASSERT_NOT_NULL( qobject_cast<BrowseWidget*>(editor) );
	
	item->setText(selector->path());
	bHandled = true;
}

// Editor to be relocated.
void FileCollectionWidget::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled)
{
	editor->setGeometry(option.rect);
	bHandled = true;
}
