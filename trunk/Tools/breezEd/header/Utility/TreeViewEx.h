#ifndef TREEVIEWEX_H
#define TREEVIEWEX_H

#include "breezEd.h"
#include <QtWidgets/QTreeView>

/// Tree widget enhanced by drag signals.
class TreeViewEx : public QTreeView
{
	Q_OBJECT

protected:
	/// Enhance dragging functionality.
	virtual void startDrag(Qt::DropActions supportedActions)
	{
		QModelIndex index = currentIndex();

		// Dragging about to begin
		Q_EMIT dragStarted(index);

		// Pass on
		QTreeView::startDrag(supportedActions);

		// Dragging finsihed
		Q_EMIT dragFinished(index);
	}

public:
	/// Constructor.
	TreeViewEx(QWidget *pParent = nullptr)
		: QTreeView(pParent) { }

Q_SIGNALS:
	/// Notifies listeners that drag/drop has started.
	void dragStarted(const QModelIndex &index);
	/// Notifies listeners that drag/drop has ended.
	void dragFinished(const QModelIndex &index);
};

#endif