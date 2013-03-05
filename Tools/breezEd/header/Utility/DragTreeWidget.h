#ifndef DRAGTREEWIDGET_H
#define DRAGTREEWIDGET_H

#include "breezEd.h"
#include <QtWidgets/QTreeWidget>

/// Tree widget enhanced by drag signals.
class DragTreeWidget : public QTreeWidget
{
	Q_OBJECT

protected:
	/// Enhance dragging functionality.
	virtual void startDrag(Qt::DropActions supportedActions)
	{
		QTreeWidgetItem *pDraggedItem = currentItem();

		// Dragging about to begin
		Q_EMIT dragStarted(pDraggedItem);

		// Pass on
		bool bPerformed = false;
		Q_EMIT performDrag(pDraggedItem, bPerformed);
		if (!bPerformed)
			QTreeWidget::startDrag(supportedActions);

		// Dragging finsihed
		Q_EMIT dragFinished(pDraggedItem);
	}

public:
	/// Constructor.
	DragTreeWidget(QWidget *pParent = nullptr)
		: QTreeWidget(pParent) { }

Q_SIGNALS:
	/// Notifies listeners that drag/drop has started.
	void dragStarted(QTreeWidgetItem *pItem);
	/// Performs drag/drop.
	void performDrag(QTreeWidgetItem *pItem, bool &bPerformed);
	/// Notifies listeners that drag/drop has ended.
	void dragFinished(QTreeWidgetItem *pItem);
};

#endif