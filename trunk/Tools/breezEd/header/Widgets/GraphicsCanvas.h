#ifndef GRAPHICSCANVAS_H
#define GRAPHICSCANVAS_H

#include <QtGui/QWidget>
#include <beGraphics/beDevice.h>
#include <lean/smart/resource_ptr.h>

#include <QtCore/QTimer>
#include <lean/time/highres_timer.h>

/// Controller builder tool.
class GraphicsCanvas : public QWidget
{
	Q_OBJECT

private:
	lean::resource_ptr<beGraphics::SwapChain> m_pSwapChain;

	QTimer m_timer;
	int m_activeFPS;
	int m_idleFPS;

	bool m_bAutoFocus;

	lean::highres_timer m_hrTimer;

protected:
	/// Intercepts show events.
	virtual void showEvent(QShowEvent *pEvent); 
	/// Intercepts hide events.
	virtual void hideEvent(QHideEvent *pEvent); 
	/// Intercepts paint events.
	virtual void paintEvent(QPaintEvent *pEvent); 
	
	/// Intercepts focus events
	virtual void focusInEvent(QFocusEvent *pEvent); 
	/// Intercepts focus events.
	virtual void focusOutEvent(QFocusEvent *pEvent); 

	/// Intercepts enter events.
	virtual void enterEvent(QEvent *pEvent); 

	/// Intercepts resize events.
	virtual void resizeEvent(QResizeEvent *pEvent); 

public:
	/// Constructor.
	GraphicsCanvas(QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~GraphicsCanvas();

	/// Sets the swap chain.
	void setSwapChain(beGraphics::SwapChain *pSwapChain);

	// Computes the optimal size of this widget.
	virtual QSize sizeHint() const;
	// Disables Qt painting on this widget.
	virtual QPaintEngine* paintEngine() const;

	/// Gets the number of FPS when focussed.
	int activeFPS() const { return m_activeFPS; }
	/// Gets the number of FPS when idle.
	int idleFPS() const { return m_idleFPS; }

	/// Gets the target FPS.
	int targetFPS() const;

	/// Gets the auto-focus property.
	bool autoFocus() const { return m_bAutoFocus; }

	/// Gets the swap chain.
	beGraphics::SwapChain* swapChain() const { return m_pSwapChain.get(); }

public Q_SLOTS:
	/// Sets the number of FPS when focussed.
	void setActiveFPS(int fps);
	/// Sets the number of FPS when idle.
	void setIdleFPS(int fps);

	/// Sets the auto-focus property.
	void setAutoFocus(bool bAutoFocus) { m_bAutoFocus = bAutoFocus; }

	/// Step & render.
	void nextFrame();
	/// Render.
	void doRender();

Q_SIGNALS:
	/// Called before any rendering is initiated.
	void step(float timeStep);
	/// Called when the canvas is repainted.
	void render();

	/// Called when the canvas recieves focus.
	void focussed(QWidget *pWidget);

	/// Called when the canvas has been resized.
	void resized(int x, int y);
};

#endif
