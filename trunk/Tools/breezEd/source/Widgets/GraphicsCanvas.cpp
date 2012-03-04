#include "stdafx.h"
#include "Widgets/GraphicsCanvas.h"

#include "Utility/Checked.h"

// Constructor.
GraphicsCanvas::GraphicsCanvas(QWidget *pParent, Qt::WFlags flags)
	: QWidget( pParent, flags | Qt::MSWindowsOwnDC ),
	m_activeFPS(60),
	m_idleFPS(30),
	m_bAutoFocus( true )
{
	// Enable direct drawing
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);

	// Accept focus & track mouse
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	// Use all space
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Enqueue frame by frame
	m_timer.setSingleShot(true);

	checkedConnect(&m_timer, SIGNAL(timeout()), this, SLOT(nextFrame())); 
}

// Destructor.
GraphicsCanvas::~GraphicsCanvas()
{
}

// Step & render.
void GraphicsCanvas::nextFrame()
{
	float timeStep = (float) m_hrTimer.seconds();

	// Measure frame time
	m_hrTimer.tick();

	// Step & render
	Q_EMIT step(timeStep);

	if (this->isVisible())
		doRender();

	// Compute time to next frame
	int targetFrameDelay = 1000 / targetFPS() - (int) m_hrTimer.milliseconds();

	// Enqueue next frame
	if (!this->isHidden())
		m_timer.start( lean::max(targetFrameDelay, 0) );
}

// Render.
void GraphicsCanvas::doRender()
{
	// Check if obscured
	if (m_pSwapChain)
	{
		// Draw scene
		Q_EMIT render();

		m_pSwapChain->Present();
	}
}

// Sets the swap chain.
void GraphicsCanvas::setSwapChain(beGraphics::SwapChain *pSwapChain)
{
	m_pSwapChain = pSwapChain;
}

// Intercepts show events.
void GraphicsCanvas::showEvent(QShowEvent *pEvent)
{
	QWidget::showEvent(pEvent);

	// Start drawing
	m_timer.start(0);
	m_hrTimer.tick();
}

// Intercepts hide events.
void GraphicsCanvas::hideEvent(QHideEvent *pEvent)
{
	QWidget::hideEvent(pEvent);

	// Stop drawing
	m_timer.stop();
}

// Intercepts paint events
void GraphicsCanvas::paintEvent(QPaintEvent *pEvent)
{
	QWidget::paintEvent(pEvent);
	
	pEvent->accept();

	if (!pEvent->region().isEmpty())
		doRender();
}

// Intercepts focus events
void GraphicsCanvas::focusInEvent(QFocusEvent *pEvent)
{
	QWidget::focusInEvent(pEvent);

	if (this->hasFocus())
		Q_EMIT focussed(this);
}

// Intercepts focus events.
void GraphicsCanvas::focusOutEvent(QFocusEvent *pEvent)
{
	QWidget::focusOutEvent(pEvent);
}

// Intercepts enter events
void GraphicsCanvas::enterEvent(QEvent *pEvent)
{
	QWidget::enterEvent(pEvent);

	// Focus on hover
	if (m_bAutoFocus)
		setFocus();
}

// Intercepts resize events
void GraphicsCanvas::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);

	Q_EMIT resized(pEvent->size().width(), pEvent->size().height());
}

// Sets the number of FPS when focussed.
void GraphicsCanvas::setActiveFPS(int fps)
{
	m_activeFPS = fps;
}

// Sets the number of FPS when idle.
void GraphicsCanvas::setIdleFPS(int fps)
{
	m_idleFPS = fps;
}

// Gets the current target FPS.
int GraphicsCanvas::targetFPS() const
{
	return (this->hasFocus())
		? m_activeFPS
		: m_idleFPS;
}

// Disables Qt painting on this widget.
QPaintEngine* GraphicsCanvas::paintEngine() const
{
	return nullptr;
}

// Gets the optimal size of this widget.
QSize GraphicsCanvas::sizeHint() const
{
	if (m_pSwapChain)
	{
		const beGraphics::SwapChainDesc &desc = m_pSwapChain->GetDesc();
		return QSize(desc.Display.Width, desc.Display.Height);
	}
	else
		return QWidget::sizeHint();
}
