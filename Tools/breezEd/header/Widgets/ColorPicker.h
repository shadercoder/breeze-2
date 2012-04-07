#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QtGui/QWidget>
#include "ui_ColorPicker.h"

#include "Utility/UI.h"

#include <beMath/beVectorDef.h>

/// Color picker tool.
class ColorPicker : public QWidget
{
	Q_OBJECT

private:
	UI<Ui::ColorPicker> ui;
	float m_highRange;

	beMath::fvec4 m_color;

public:
	/// Constructor.
	ColorPicker(float highRange, QWidget *pParent = nullptr);
	/// Destructor.
	~ColorPicker();

	/// Gets the color.
	beMath::fvec4 color() const;

public Q_SLOTS:
	/// Sets the color.
	void setColor(const beMath::fvec4 &color);

	/// Paints the color wheel.
	void paintWheel(QPainter &painter);
	/// Paints the saturation slider.
	void paintValue(QPainter &painter);
	/// Paints the alpha slider.
	void paintAlpha(QPainter &painter);

	/// Selects a wheel color.
	void wheelSelect(float x, float y, QEvent::Type type);
	/// Selects a brightness.
	void valueSelect(float x, float y, QEvent::Type type);
	/// Selects a transparency.
	void alphaSelect(float x, float y, QEvent::Type type);

Q_SIGNALS:
	/// Called whenever the color is changed.
	void colorChanged(const beMath::fvec4 &color);
};

#endif
