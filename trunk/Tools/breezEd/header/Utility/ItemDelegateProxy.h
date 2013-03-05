#ifndef ITEMDELEGATEPROXY_H
#define ITEMDELEGATEPROXY_H

#include "breezEd.h"
#include <QtWidgets/QAbstractItemDelegate>

/// Item delegate enhanced by custom editing signals.
class ItemDelegateProxy : public QAbstractItemDelegate
{
	Q_OBJECT

private:
	QAbstractItemDelegate *m_wrapped;

public:
	/// Constructor.
	ItemDelegateProxy(QAbstractItemDelegate *wrapped, QObject *pParent = nullptr)
		: QAbstractItemDelegate(pParent),
		m_wrapped(LEAN_ASSERT_NOT_NULL(wrapped))
	{
		// TODO
//		connect(this, &QAbstractItemDelegate::commitData, m_wrapped, &QAbstractItemDelegate::commitData);
//		connect(this, &QAbstractItemDelegate::closeEditor, m_wrapped, &QAbstractItemDelegate::closeEditor);
//		connect(this, &QAbstractItemDelegate::sizeHintChanged, m_wrapped, &QAbstractItemDelegate::sizeHintChanged);

		connect(m_wrapped, &QAbstractItemDelegate::commitData, this, &QAbstractItemDelegate::commitData);
		connect(m_wrapped, &QAbstractItemDelegate::closeEditor, this, &QAbstractItemDelegate::closeEditor);
		connect(m_wrapped, &QAbstractItemDelegate::sizeHintChanged, this, &QAbstractItemDelegate::sizeHintChanged);
	}

	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->createEditor(parent, option, index);
	}
	void destroyEditor(QWidget *editor, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->destroyEditor(editor, index);
	}
	void setEditorData(QWidget *editor, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->setEditorData(editor, index);
	}
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->setModelData(editor, model, index);
	}
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->updateEditorGeometry(editor, option, index);
	}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->paint(painter, option, index);
	}
	QVector<int> paintingRoles() const
	{
		return m_wrapped->paintingRoles();
	}
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const LEAN_OVERRIDE
	{
		return m_wrapped->sizeHint(option, index);
	}

	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) LEAN_OVERRIDE
	{
		return m_wrapped->editorEvent(event, model, option, index);
	}
	bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) LEAN_OVERRIDE
	{
		return m_wrapped->helpEvent(event, view, option, index);
	}

	bool eventFilter(QObject *watched, QEvent *event)
	{
		return m_wrapped->eventFilter(watched, event);
	}

	QAbstractItemDelegate* wrapped() const { return m_wrapped; }
};

#endif