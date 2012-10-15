#include "stdafx.h"
#include "Widgets/ColorPicker.h"

#include <QtGui/QLinearGradient>

#include "Utility/Checked.h"

namespace
{

QColor fromRGB(const beMath::fvec4 &color, float *pValueExp = nullptr)
{
	float valueExp = max(max(color[0], color[1]), max(color[2], 1.0f));

	if (pValueExp)
		*pValueExp = valueExp;

	return QColor::fromRgbF(
			min(color[0] / valueExp, 1.0f),
			min(color[1] / valueExp, 1.0f),
			min(color[2] / valueExp, 1.0f),
			min(color[3], 1.0f)
		);
}

beMath::fvec4 toRGB(const QColor &color, float valueExp = 1.0f)
{
	return beMath::vec<float>(
			(float) color.redF() * valueExp,
			(float) color.greenF() * valueExp,
			(float) color.blueF() * valueExp,
			(float) color.alphaF()
		);
}

#ifndef PAINTERS_CIRCLE

float toHue(float hue)
{
	if (hue <= 240.0f / 480.0f)
		hue *= 0.5f;
	else
		hue -= 120.0f / 480.0f;

	return hue * (480.0f / 360.0f);
}

float toPHue(float hue)
{
	if (hue <= 120.0f / 360.0f)
		hue *= 2.0f;
	else
		hue += 120.0f / 360.0f;

	return hue * (360.0f / 480.0f);
}

#else

float toHue(float hue)
{
	if (hue <= 120.0f / 360.0f)
		hue *= 0.5f;
	else if (hue <= 240.0f / 360.0f)
		hue = (hue - 120.0f / 360.0f) * (180.0f / 120.0f) + 60.0f / 360.0f;

	return hue;
}

float toPHue(float hue)
{
	if (hue <= 60.0f / 360.0f)
		hue *= 2.0f;
	else if (hue <= 240.0f / 360.0f)
		hue = (hue - 60.0f / 360.0f) * (120.0f / 180.0f) + 120.0f / 360.0f;

	return hue;
}

#endif

QColor blend(const QColor &src, const QColor &dest)
{
	return QColor::fromRgbF(
			dest.redF() + (src.redF() - dest.redF()) * src.alphaF(),
			dest.greenF() + (src.greenF() - dest.greenF()) * src.alphaF(),
			dest.blueF() + (src.blueF() - dest.blueF()) * src.alphaF(),
			dest.alphaF() + (1.0f - dest.alphaF()) * src.alphaF()
		);
}

void drawMarker(QPainter &painter, float x, float y, const QColor &color)
{
	QRect window = painter.window();
	QPoint center(
			window.x() + (int) (window.width() * x),
			window.y() + (int) (window.height() * y)
		);

	QColor markerColor = (color.valueF() > 0.35f) ? Qt::black : Qt::gray;

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(QBrush());
	painter.setPen(QPen(markerColor));
	painter.drawEllipse(center, 6, 6);
}

float toHR(float value, float &valueExp, float highRange)
{
	valueExp = 1.0f;

	if (highRange > 1.0f)
	{
		value *= 2.0f;

		if (value > 1.0f)
		{
			valueExp = pow(highRange, value - 1.0f);
			value = 1.0f;
		}
	}

	return value;
}

float fromHR(float value, float highRange)
{
	if (highRange > 1.0f)
	{
		if (value > 1.0f)
			value = 1.0f + log(value) / log(highRange);

		value *= 0.5f;
	}

	return value;
}

} // namespace

// Constructor.
ColorPicker::ColorPicker(float highRange, QWidget *pParent)
	: QWidget(pParent),
	ui(this),
	m_highRange(highRange),
	m_color( beMath::vec(0.8f, 0.5f, 0.1f, 1.0f) )
{
	checkedConnect(ui.colorFrame, SIGNAL(touched(float, float, QEvent::Type)), this, SLOT(wheelSelect(float, float, QEvent::Type)), Qt::DirectConnection);
	checkedConnect(ui.valueFrame, SIGNAL(touched(float, float, QEvent::Type)), this, SLOT(valueSelect(float, float, QEvent::Type)), Qt::DirectConnection);
	checkedConnect(ui.alphaFrame, SIGNAL(touched(float, float, QEvent::Type)), this, SLOT(alphaSelect(float, float, QEvent::Type)), Qt::DirectConnection);

	checkedConnect(ui.colorFrame, SIGNAL(paint(QPainter&)), this, SLOT(paintWheel(QPainter&)), Qt::DirectConnection);
	checkedConnect(ui.valueFrame, SIGNAL(paint(QPainter&)), this, SLOT(paintValue(QPainter&)), Qt::DirectConnection);
	checkedConnect(ui.alphaFrame, SIGNAL(paint(QPainter&)), this, SLOT(paintAlpha(QPainter&)), Qt::DirectConnection);
}

// Destructor.
ColorPicker::~ColorPicker()
{
}

// Sets the color.
void ColorPicker::setColor(const beMath::fvec4 &color)
{
	m_color = color;

	ui.colorFrame->repaint();
	ui.valueFrame->repaint();
	ui.alphaFrame->repaint();

	Q_EMIT colorChanged(m_color);
}

// Gets the color.
beMath::fvec4 ColorPicker::color() const
{
	return m_color;
}

// Selects a wheel color.
void ColorPicker::wheelSelect(float x, float y, QEvent::Type type)
{
	float hue = toHue( min(max(0.0f, x), 1.0f) );
	float saturation = min(max(0.0f, 1.0f - y), 1.0f);

	float valueExp;
	QColor baseColor = fromRGB(m_color, &valueExp);

	QColor color = QColor::fromHsvF(hue, saturation, baseColor.valueF(), baseColor.alphaF());

	m_color = toRGB(color, valueExp);
	
	if (type != QEvent::MouseMove)
		ui.colorFrame->repaint();
	ui.valueFrame->repaint();
	ui.alphaFrame->repaint();

	Q_EMIT colorChanged(m_color);
}

// Selects a brightness.
void ColorPicker::valueSelect(float x, float y, QEvent::Type type)
{
	float value = min(max(0.0f, 1.0f - y), 1.0f);
	float valueExp;
	value = toHR(value, valueExp, m_highRange);

	QColor baseColor = fromRGB(m_color);

	QColor color = QColor::fromHsvF(baseColor.hueF(), baseColor.saturationF(), value, baseColor.alphaF());

	m_color = toRGB(color, valueExp);

	ui.colorFrame->repaint();
	if (type != QEvent::MouseMove)
		ui.valueFrame->repaint();
	ui.alphaFrame->repaint();

	Q_EMIT colorChanged(m_color);
}

// Selects a transparency.
void ColorPicker::alphaSelect(float x, float y, QEvent::Type type)
{
	float alpha = min(max(0.0f, 1.0f - y), 1.0f);

	m_color[3] = alpha;

	if (type != QEvent::MouseMove)
		ui.alphaFrame->repaint();

	Q_EMIT colorChanged(m_color);
}

// Paints the color wheel.
void ColorPicker::paintWheel(QPainter &painter)
{
	QColor baseColor = fromRGB(m_color);
	float value = baseColor.valueF();

	QLinearGradient hueGradient(0.0f, 0.5f, 1.0f, 0.5f);
	
	for (int i = 0; i <= 480; i += 60)
	{
		float hue = toHue(i / 480.0f);
		hueGradient.setColorAt(i / 480.0f, QColor::fromHsvF(hue, 1.0f, value));
	}
	hueGradient.setCoordinateMode(QGradient::StretchToDeviceMode);

	painter.fillRect(painter.window(), QBrush(hueGradient));

	QLinearGradient saturationGradient(0.5f, 0.0f, 0.5f, 1.0f);
	saturationGradient.setColorAt(0.0f, QColor::fromHsvF(0.0f, 0.0f, value, 0.0f));
	saturationGradient.setColorAt(1.0f, QColor::fromHsvF(0.0f, 0.0f, value, 1.0f));
	saturationGradient.setCoordinateMode(QGradient::StretchToDeviceMode);

	painter.fillRect(painter.window(), QBrush(saturationGradient));

	if (!ui.colorFrame->isTouched())
		drawMarker(painter, toPHue(baseColor.hueF()), 1.0f - baseColor.saturationF(), baseColor);
}

// Paints the saturation slider.
void ColorPicker::paintValue(QPainter &painter)
{
	float valueExp;
	QColor baseColor = fromRGB(m_color, &valueExp);
	float hue = baseColor.hueF();
	float saturation = baseColor.saturationF();

	QLinearGradient brightnessGradient(0.5f, (m_highRange > 1.0f) ? 0.5f : 0.0f, 0.5f, 1.0f);
	brightnessGradient.setColorAt(0.0f, QColor::fromHsvF(hue, saturation, 1.0f));
	brightnessGradient.setColorAt(1.0f, QColor::fromHsvF(hue, saturation, 0.0f));
	brightnessGradient.setCoordinateMode(QGradient::StretchToDeviceMode);

	painter.fillRect(painter.window(), QBrush(brightnessGradient));

	if (!ui.valueFrame->isTouched())
		drawMarker(painter, 0.5f, 1.0f - fromHR(baseColor.valueF() * valueExp, m_highRange), baseColor);
}

// Paints the alpha slider.
void ColorPicker::paintAlpha(QPainter &painter)
{
	QPixmap checkerTile(16, 16);
	
	{
		checkerTile.fill(Qt::white);

		QPainter checkerPainter(&checkerTile);
		checkerPainter.fillRect(0, 0, 8, 8, QBrush(Qt::gray));
		checkerPainter.fillRect(8, 8, 16, 16, QBrush(Qt::gray));
	}

	painter.fillRect(painter.window(), QBrush(checkerTile));

	QColor baseColor = fromRGB(m_color);
	float hue = baseColor.hueF();
	float saturation = baseColor.saturationF();
	float brightness = baseColor.valueF();

	QLinearGradient alphaGradient(0.5f, 0.0f, 0.5f, 1.0f);
	alphaGradient.setColorAt(0.0f, QColor::fromHsvF(hue, saturation, brightness, 1.0f));
	alphaGradient.setColorAt(1.0f, QColor::fromHsvF(hue, saturation, brightness, 0.0f));
	alphaGradient.setCoordinateMode(QGradient::StretchToDeviceMode);

	painter.fillRect(painter.window(), QBrush(alphaGradient));

	if (!ui.alphaFrame->isTouched())
		drawMarker(painter, 0.5f, 1.0f - baseColor.alphaF(), blend(baseColor, Qt::white));
}
