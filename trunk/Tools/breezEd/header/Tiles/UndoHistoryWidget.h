#ifndef UNDOHISTORYWIDGET_H
#define UNDOHISTORYWIDGET_H

#include "ui_UndoHistoryWidget.h"

class QUndoStack;
class AbstractDocument;

/// Console tool.
class UndoHistoryWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::UndoHistoryWidget ui;

public:
	/// Constructor.
	UndoHistoryWidget(QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~UndoHistoryWidget();

	/// Gets the undo stack.
	QUndoStack* undoStack();
	/// Gets the undo stack.
	const QUndoStack* undoStack() const;

public Q_SLOTS:
	/// Sets the undo stack.
	void setUndoStack(QUndoStack *pStack);
	/// Sets the undo stack of the given document.
	void setDocument(AbstractDocument *pDocument);
};

#endif
