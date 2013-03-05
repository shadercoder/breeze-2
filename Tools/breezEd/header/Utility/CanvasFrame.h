#ifndef CANVASFRAME_H
#define CANVASFRAME_H

#include "breezEd.h"
#include <QtWidgets/QFrame>
#include <QtGui/QPainter>

/// Square frame.
class CanvasFrame : public QFrame
{
	Q_OBJECT

private:
	bool m_bMousePressed;

	/// Emits touched signals.
	void emitTouched(const QPointF &pos, QEvent::Type type)
	{
		QRect contentRect = this->contentsRect();

		float x = (pos.x() - contentRect.x()) / contentRect.width();
		float y = (pos.y() - contentRect.y()) / contentRect.height();
		
		Q_EMIT touched(x, y, type);
	}

protected:
	/// Intercepts paint events.
	virtual void paintEvent(QPaintEvent *event)
	{
		QFrame::paintEvent(event);

		QPainter painter(this);
		painter.setClipRegion(event->region());
		painter.setViewport(this->contentsRect());

		Q_EMIT paint(painter);
	}

	/// Intercepts mouse events.
	virtual void mousePressEvent(QMouseEvent *event)
	{
		QFrame::mousePressEvent(event);

		if (event->button() == Qt::LeftButton)
		{
			m_bMousePressed = true;
			emitTouched(event->pos(), QEvent::MouseButtonPress);
		}
	}

	/// Intercepts mouse events.
	virtual void mouseMoveEvent(QMouseEvent *event)
	{
		QFrame::mouseMoveEvent(event);

		if (event->buttons() & Qt::LeftButton)
			emitTouched(event->pos(), QEvent::MouseMove);
	}

	/// Intercepts mouse events.
	virtual void mouseReleaseEvent(QMouseEvent *event)
	{
		QFrame::mouseReleaseEvent(event);

		if (event->button() == Qt::LeftButton)
		{
			m_bMousePressed = false;
			emitTouched(event->pos(), QEvent::MouseButtonRelease);
		}
	}

public:
	/// Constructor.
	CanvasFrame(QWidget *pParent = nullptr, Qt::WindowFlags f = 0)
		: QFrame(pParent, f),
		m_bMousePressed(false)
	{
	}

	/// True, if the mouse is currently pressed.
	bool isTouched() { return m_bMousePressed; }

Q_SIGNALS:
	/// Paint.
	void paint(QPainter &painter);
	/// Touched.
	void touched(float x, float y, QEvent::Type type = QEvent::MouseMove);
};

/// Square frame.
class SquareCanvasFrame : public CanvasFrame
{
	Q_OBJECT

public:
	/// Constructor.
	SquareCanvasFrame(QWidget *pParent = nullptr, Qt::WindowFlags f = 0)
		: CanvasFrame(pParent, f)
	{
		QSizePolicy sizePolicy(this->sizePolicy());
		sizePolicy.setHeightForWidth(true);
		this->setSizePolicy(sizePolicy);
	}

	/// Returns width.
	virtual int heightForWidth(int width) const
	{
		return width;
	}
};

#endif