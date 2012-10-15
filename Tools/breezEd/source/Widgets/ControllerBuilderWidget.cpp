#include "stdafx.h"
#include "Widgets/ControllerBuilderWidget.h"

#include <beEntitySystem/beControllerSerializer.h>
#include <beEntitySystem/beControllerSerialization.h>

#include <beCore/beComponentTypes.h>
#include <beCore/beComponentReflector.h>

#include "Plugins/FactoryManager.h"
#include "Widgets/ComponentPickerFactory.h"
#include "Widgets/ComponentPicker.h"

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
void addCreationParameters(Ui::ControllerBuilderWidget &widget, const beEntitySystem::ControllerSerializer *serializer, Editor *editor)
{
	const beEntitySystem::ControllerSerializer::SerializationParameters &creationParameters = serializer->GetCreationParameters();

	for (const beEntitySystem::CreationParameter *it = creationParameters.begin(); it != creationParameters.end(); ++it)
	{
		QString parameterName = toQt(it->Name);
		QString parameterType = toQt(it->Type);

		QGroupBox *parameterGroup = new QGroupBox(parameterName, widget.centerWidget);
		QVBoxLayout *layout = new QVBoxLayout(parameterGroup);
		layout->setMargin(6);

		if (it->Optional)
		{
			parameterGroup->setCheckable(true);
			parameterGroup->setChecked(true);
		}

		const beCore::ComponentReflector *pReflector = beCore::GetComponentTypes().GetReflector(it->Type);

		QWidget *parameterWidget;

		if (pReflector)
		{
			const ComponentPickerFactory &componentPickerFactory = *LEAN_ASSERT_NOT_NULL(
					getComponentPickerFactories().getFactory(parameterType)
				);

			parameterWidget = componentPickerFactory.createComponentPicker(pReflector, nullptr, editor, parameterGroup);
		}
		else
			parameterWidget = new QLabel(
					ControllerBuilderWidget::tr("Unknown type '%1'").arg(parameterType),
					parameterGroup
				);

		layout->addWidget(parameterWidget);

		parameterGroup->setLayout(layout);
		widget.centerLayout->addWidget(parameterGroup);
	}

	if (creationParameters.empty())
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
	int parameterCount = ui.centerLayout->count();
	
	for (int idx = 0; idx < parameterCount; ++idx)
	{
		QGroupBox *pParameterGroup = qobject_cast<QGroupBox*>( ui.centerLayout->itemAt(idx)->widget() );
		
		if (pParameterGroup && (!pParameterGroup->isCheckable() || pParameterGroup->isChecked()))
		{
			lean::utf8_string parameterName = toUtf8(pParameterGroup->title());
			QWidget &parameterWidget = *LEAN_ASSERT_NOT_NULL( pParameterGroup->layout()->itemAt(0)->widget() );

			ComponentPicker *pComponentPicker = qobject_cast<ComponentPicker*>(&parameterWidget);

			if (pComponentPicker)
				parameters.SetAnyValue( parameters.Add(parameterName), &*pComponentPicker->acquireComponent(document) );
		}
	}
}
