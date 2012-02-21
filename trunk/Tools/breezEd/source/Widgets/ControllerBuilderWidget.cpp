#include "stdafx.h"
#include "Widgets/ControllerBuilderWidget.h"

#include <beEntitySystem/beControllerSerializer.h>
#include <beEntitySystem/beControllerSerialization.h>

#include "Plugins/WidgetFactoryManager.h"
#include "Widgets/CreationParameterWidgetFactory.h"

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
void addCreationParameters(Ui::ControllerBuilderWidget &widget, const beEntitySystem::ControllerSerializer *pSerializer, Editor *pEditor, QList<CreationParameterWidget*> &parameters)
{
	const beEntitySystem::ControllerSerializer::SerializationParameters &creationParameters = pSerializer->GetCreationParameters();

	if (!creationParameters.empty())
	{
		const WidgetFactoryManager<CreationParameterWidgetFactory> &factories = getCreationParameterWidgetFactory();

		for (const beEntitySystem::CreationParameter *it = creationParameters.begin(); it != creationParameters.end(); ++it)
		{
			QString parameterName = QString::fromUtf8(it->Name.c_str());
			QString parameterType = QString::fromUtf8(it->Type.c_str());

			const CreationParameterWidgetFactory *pFactory = factories.getFactory(parameterType);
			CreationParameterWidget *pParameter = (pFactory) ? pFactory->createWidget(parameterName, pEditor, widget.centerWidget) : nullptr;

			QWidget *pParameterWidget;

			if (pParameter)
			{
				parameters.push_back(pParameter);
				pParameterWidget = pParameter->widget();
			}
			else
				pParameterWidget = new QLabel(
					ControllerBuilderWidget::tr("Unknown type '%1'").arg(parameterType),
					widget.centerWidget );

			widget.centerLayout->addRow(parameterName + ":", pParameterWidget);
		}
	}
	else
		widget.centerWidget->hide();
}

} // namespace

// Constructor.
ControllerBuilderWidget::ControllerBuilderWidget(const beEntitySystem::ControllerSerializer *pSerializer, Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pSerializer( LEAN_ASSERT_NOT_NULL(pSerializer) )
{
	ui.setupUi(this);

	ui.nameLabel->setText( QString::fromUtf8(pSerializer->GetType().c_str()) );

	randomizeBackground(*this);

	addCreationParameters(ui, m_pSerializer, pEditor, m_parameters);

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
	Q_FOREACH (CreationParameterWidget *pParameter, m_parameters)
		pParameter->setParameters(parameters, document);
}
