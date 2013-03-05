#ifndef INLINEEDITOR_H
#define INLINEEDITOR_H

#include "breezEd.h"
#include <QtWidgets/QFrame>

class InlineEditor : public QFrame
{
	Q_OBJECT

private:
	bool m_bOverlay;

public:
	InlineEditor(bool bOverlay, QWidget *pParent)
		: QFrame(pParent),
		m_bOverlay(bOverlay)
	{
		// Opaque for mouse and eyes
		this->setAttribute(Qt::WA_NoMousePropagation);
		this->setAutoFillBackground(true);

		QVBoxLayout *layout = new QVBoxLayout(this);
		
		if (m_bOverlay)
		{
			this->setFrameShape(QFrame::StyledPanel);
			layout->setMargin(3);
		}
		else
			layout->setMargin(0);
	}

	void setWidget(QWidget *widget)
	{
		QWidget *prevWidget = this->widget();
		if (widget == prevWidget)
			return;

		delete prevWidget;

		this->layout()->addWidget(widget);
		this->setFocusProxy(widget);

		if (m_bOverlay)
			this->adjustSize();
	}

	QWidget* widget() const
	{
		QLayoutItem *item = this->layout()->itemAt(0);
		return (item) ? item->widget() : nullptr;
	}
	template <class T>
	T* widget() const { return qobject_cast<T*>(this->widget()); }

	void connectToDelegate(QAbstractItemDelegate *delegate)
	{
		connect(this, &InlineEditor::commitData, delegate, &QAbstractItemDelegate::commitData);
		connect(this, &InlineEditor::closeEditor, delegate, &QAbstractItemDelegate::closeEditor);
	}

	bool isOverlay() const { return m_bOverlay; }

public Q_SLOTS:
	virtual void accept() { }

	void commitAndClose() { Q_EMIT commitData(this); Q_EMIT closeEditor(this); }
	void abortAndClose() { Q_EMIT closeEditor(this); }

Q_SIGNALS:
	void commitData(QWidget*);
	void closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint = QAbstractItemDelegate::NoHint);
};

#endif