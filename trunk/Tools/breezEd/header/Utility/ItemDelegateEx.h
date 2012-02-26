#ifndef ITEMDELEGATEEX_H
#define ITEMDELEGATEEX_H

#include "breezEd.h"
#include <QtGui/QStyledItemDelegate>

/// Item delegate enhanced by custom editing signals.
class ItemDelegateEx : public QStyledItemDelegate
{
	Q_OBJECT

public:
	/// Constructor.
	ItemDelegateEx(QWidget *pParent = nullptr)
		: QStyledItemDelegate(pParent) { }

	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		bool bHandled = false;
		QWidget *pEditor = nullptr;

		Q_EMIT startEditing(parent, option, index, pEditor, bHandled);

		if (!bHandled && !pEditor)
			pEditor = QStyledItemDelegate::createEditor(parent, option, index);

		return pEditor;
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		bool bHandled = false;

		Q_EMIT updateEditor(editor, index, bHandled);

		if (!bHandled)
			QStyledItemDelegate::setEditorData(editor, index);
	}

	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		bool bHandled = false;

		Q_EMIT updateData(editor, model, index, bHandled);

		if (!bHandled)
			QStyledItemDelegate::setModelData(editor, model, index);

		Q_EMIT dataUpdated(model, index);
	}

	virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		bool bHandled = false;

		Q_EMIT updateEditorGeometry(editor, option, index, bHandled);

		if (!bHandled)
			QStyledItemDelegate::updateEditorGeometry(editor, option, index);
	}

Q_SIGNALS:
	/// Notifies listeners that an editor should be created.
	void startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled) const;
	/// Notifies listeners that the given editor should be updated.
	void updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled) const;
	/// Notifies listeners that data from the given editor should be committed.
	void updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled) const;
	/// Notifies listeners that new data has been committed.
	void dataUpdated(QAbstractItemModel *model, const QModelIndex &index) const;
	/// Notifies listeners that the editor should be relocated.
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled) const;
};

#endif