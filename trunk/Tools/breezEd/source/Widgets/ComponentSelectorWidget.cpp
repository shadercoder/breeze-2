#include "stdafx.h"
#include "Widgets/ComponentSelectorWidget.h"

#include <QtGui/QFileDialog>
#include <QtCore/QDir>

#include <beCore/beComponentTypes.h>

#include "Documents/SceneDocument.h"

#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

// Adapts the UI to match the given reflector.
void AdaptUI(ComponentSelectorWidget &selector, Ui::ComponentSelectorWidget &ui, const beCore::ComponentReflector *pReflector)
{
	int optionCount = 0;

	// Remove file option, if not available
	if (!pReflector->CanBeLoaded())
	{
		ui.fileButton->hide();
		ui.formLayout->removeWidget(ui.fileButton);

		ui.browseWidget->hide();
		ui.formLayout->removeWidget(ui.browseWidget);
	}
	else
	{
		ui.fileButton->setChecked(optionCount == 0);
		++optionCount;
	}

	// Remove name option, if not available
	if (!pReflector->CanBeNamed())
	{
		ui.nameButton->hide();
		ui.formLayout->removeWidget(ui.nameButton);

		ui.nameEdit->hide();
		ui.formLayout->removeWidget(ui.nameEdit);
	}
	else
	{
		ui.nameButton->setChecked(optionCount == 0);
		++optionCount;
	}

	// Recursively build parameter list
	if (pReflector->CanBeCreated())
	{
		beCore::ComponentParameters parameters = pReflector->GetCreationParameters();

		for (beCore::ComponentParameters::iterator it = parameters.begin(); it != parameters.end(); ++it)
		{
			QGroupBox *parameterGroup = new QGroupBox( toQt(it->Name), &selector );
			QVBoxLayout *layout = new QVBoxLayout(parameterGroup);

			if (it->Type == "String")
				layout->addWidget( new QLineEdit(parameterGroup) );
			else
			{
				const beCore::ComponentReflector *pReflector = beCore::GetComponentTypes().GetReflector(it->Type);

				if (pReflector)
					layout->addWidget( new ComponentSelectorWidget(pReflector, parameterGroup) );
				else
					layout->addWidget( new QLabel(
							ComponentSelectorWidget::tr("Unknown type '%1'").arg( toQt(it->Type) ),
							parameterGroup
						) );
			}

			parameterGroup->setLayout(layout);
			ui.newLayout->addWidget(parameterGroup);
		}

		ui.newButton->setChecked(optionCount == 0);
		++optionCount;
	}
	// Remove new option, if not available
	else
	{
		ui.newButton->hide();
		ui.formLayout->removeWidget(ui.newButton);

		ui.formLayout->removeItem(ui.newLayout);
		ui.newLayout->deleteLater();
		ui.newLayout = nullptr;
	}

	// Don't show labels, if only one option
	if (optionCount == 1)
	{
		ui.fileButton->hide();
		ui.nameButton->hide();
		ui.newButton->hide();
	}

	selector.adjustSize();
}

} // namespace

// Constructor.
ComponentSelectorWidget::ComponentSelectorWidget(const beCore::ComponentReflector *pReflector, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pReflector( LEAN_ASSERT_NOT_NULL(pReflector) )
{
	ui.setupUi(this);

	AdaptUI(*this, ui, m_pReflector);

	checkedConnect(ui.nameEdit, SIGNAL(cursorPositionChanged(int, int)), ui.nameButton, SLOT(click()));
	
	checkedConnect(ui.browseWidget, SIGNAL(editingStarted()), ui.fileButton, SLOT(click()));
	checkedConnect(ui.browseWidget, SIGNAL(browse()), this, SLOT(browse()));
}

// Destructor.
ComponentSelectorWidget::~ComponentSelectorWidget()
{
}

// Browser requested.
void ComponentSelectorWidget::browse()
{
	// Open either breeze mesh or importable 3rd-party mesh format
	QString file = QFileDialog::getOpenFileName( this,
			tr("Select a ''%1' resource").arg( toQt(m_pReflector->GetType()) ),
			QDir::currentPath(), // TODO
			QString("%1 %2 (*.%3);;%4 (*.*)")
				.arg( toQt(m_pReflector->GetType()) )
				.arg( tr("Files") )
				.arg( toQt(m_pReflector->GetFileExtension()) )
				.arg( tr("All Files") )
		);

	if (!file.isEmpty())
	{
		ui.browseWidget->setPath(file);
		ui.fileButton->setChecked(true);
	}
}

// Gets the selected component.
lean::cloneable_obj<lean::any> ComponentSelectorWidget::acquireComponent(SceneDocument &document) const
{
	beCore::ParameterSet parameters = document.getSerializationParameters();

	if (ui.fileButton->isChecked())
		return *m_pReflector->GetComponent( toUtf8Range(ui.browseWidget->path()), parameters );
	else if (ui.nameButton->isChecked())
		return *m_pReflector->GetComponentByName( toUtf8Range(ui.nameEdit->text()), parameters );
	else
	{
		beCore::Parameters creationParameters;

		int parameterCount = ui.newLayout->count();

		for (int idx = 0; idx < parameterCount; ++idx)
		{
			QGroupBox &groupBox = *LEAN_ASSERT_NOT_NULL( qobject_cast<QGroupBox*>( ui.newLayout->itemAt(idx)->widget() ) );

			lean::utf8_string parameterName = toUtf8( groupBox.title() );
			QWidget &parameterWidget = *LEAN_ASSERT_NOT_NULL( groupBox.layout()->itemAt(0)->widget() );

			QLineEdit *pString = qobject_cast<QLineEdit*>(&parameterWidget);

			if (pString)
				creationParameters.SetValue<beCore::Exchange::utf8_string>(
						creationParameters.Add(parameterName),
						toUtf8Range(pString->text()).get().to<beCore::Exchange::utf8_string>()
					);
			else
			{
				ComponentSelectorWidget *pSubComponent = qobject_cast<ComponentSelectorWidget*>(&parameterWidget);

				if (pSubComponent)
					creationParameters.SetAnyValue( creationParameters.Add(parameterName), &*pSubComponent->acquireComponent(document) );
			}
		}

		return *m_pReflector->CreateComponent( creationParameters, parameters );
	}
}
