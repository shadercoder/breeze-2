#include "stdafx.h"
#include "Widgets/ControllerBuilderWidget.h"

#include <beEntitySystem/beControllerSerializer.h>
#include <beEntitySystem/beSerialization.h>

#include <beCore/beComponentTypes.h>
#include <beCore/beComponentReflector.h>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

/// Yiels a random integer between a and b.
int rand(int a, int b)
{
	return a + ::rand() * (b - a) / RAND_MAX;
}

/// Randomizes the background color of the given widget.
void randomizeBackground(QWidget &widget)
{
	QPalette backPalette;
	QBrush backBrush = widget.palette().background();
	QColor backColor = backBrush.color();
	backColor.setRgb(backColor.red() + rand(-15, 15), backColor.green() + rand(-15, 15), backColor.blue() + rand(-15, 15));
	backBrush.setColor(backColor);
	backPalette.setBrush(QPalette::Background, backBrush);
	widget.setBackgroundRole(QPalette::Background);
	widget.setPalette(backPalette);
	widget.setAutoFillBackground(true);
}

/// Adds creation parameters to the given controller builder widget.
void addCreationParameters(Ui::ControllerBuilderWidget &ui, const beEntitySystem::ControllerSerializer *serializer, Editor *editor)
{
	{
		lean::scoped_ptr<ComponentParameterWidget> parameterWidget( new ComponentParameterWidget(
				serializer->GetCreationParameters(), bec::Parameters(), false, editor, ui.parameterBox
			) );
		ui.parameterBox->layout()->addWidget(parameterWidget);
		delete ui.parameterWidget;
		ui.parameterWidget = parameterWidget.detach();
	}

	ui.parameterBox->setVisible(ui.parameterWidget->hasParameters());
}

} // namespace

// Constructor.
ControllerBuilderWidget::ControllerBuilderWidget(const beEntitySystem::ControllerSerializer *pSerializer, Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pSerializer( LEAN_ASSERT_NOT_NULL(pSerializer) )
{
	ui.setupUi(this);

	ui.nameLabel->setText( QString::fromUtf8(pSerializer->GetType().c_str()) );

	randomizeBackground(*this);

	addCreationParameters(ui, m_pSerializer, pEditor);

	checkedConnect(ui.upButton, SIGNAL(clicked()), this, SIGNAL(moveUp()));
	checkedConnect(ui.downButton, SIGNAL(clicked()), this, SIGNAL(moveDown()));
}

// Destructor.
ControllerBuilderWidget::~ControllerBuilderWidget()
{
}

// Sets the creation parameter in the given set.
void ControllerBuilderWidget::setParameters(beCore::Parameters &parameters, SceneDocument &document) const
{
	ui.parameterWidget->getParameters(parameters);
}
