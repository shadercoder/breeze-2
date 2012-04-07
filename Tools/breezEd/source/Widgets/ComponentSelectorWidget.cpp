#include "stdafx.h"
#include "Widgets/ComponentSelectorWidget.h"

#include <QtGui/QFileDialog>
#include <QtCore/QDir>

#include <beCore/beComponentTypes.h>

#include "Documents/SceneDocument.h"

#include "Editor.h"

#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

/// Hides & removes the given widget from its layout.
void hideAndRemove(QWidget &widget)
{
	widget.hide();

	QWidget *pParent = widget.parentWidget();

	if (pParent)
	{
		QLayout *pLayout = pParent->layout();

		if (pLayout)
			pLayout->removeWidget(&widget);
	}
}

/// Recursively retrieves the last focus proxy in a chain.
QWidget* lastFocusProxy(QWidget *widget)
{
	QWidget *proxy = widget, *nextProxy = widget;

	while (nextProxy)
	{
		proxy = nextProxy;
		nextProxy = proxy->focusProxy();
	}

	return proxy;
}

/// Recursively checks if the given widget is a child of the given parent widget.
bool isChild(QWidget *widget, QWidget *parent)
{
	QWidget *intermediateParent = widget;

	while (intermediateParent)
	{
		if (intermediateParent == parent)
			return true;

		intermediateParent = intermediateParent->parentWidget();
	}

	return false;
}

/// Adapts the UI to match the given reflector.
void adaptUI(ComponentSelectorWidget &selector, Ui::ComponentSelectorWidget &ui, const beCore::ComponentReflector &reflector, Editor *pEditor)
{
	int optionCount = 0;

	// Remove file option, if not available
	if (!reflector.CanBeLoaded())
	{
		hideAndRemove(*ui.fileButton);
		hideAndRemove(*ui.browseWidget);
	}
	else
	{
		ui.browseWidget->installFocusHandler(&selector);

		if (optionCount == 0)
		{
			ui.fileButton->setChecked(true);
			selector.setFocusProxy(ui.browseWidget);
		}
		++optionCount;
	}

	// Remove name option, if not available
	if (!reflector.HasName())
	{
		hideAndRemove(*ui.nameButton);
		hideAndRemove(*ui.nameEdit);
	}
	else
	{
		ui.nameEdit->installEventFilter(&selector);

		if (optionCount == 0)
		{
			ui.nameButton->setChecked(true);
			selector.setFocusProxy(ui.nameEdit);
		}
		++optionCount;
	}

	// Recursively build parameter list
	if (reflector.CanBeCreated())
	{
		QWidget *firstParameterWidget = nullptr;

		beCore::ComponentParameters parameters = reflector.GetCreationParameters();

		for (beCore::ComponentParameters::iterator it = parameters.begin(); it != parameters.end(); ++it)
		{
			QGroupBox *parameterGroup = new QGroupBox( toQt(it->Name), &selector );
			QVBoxLayout *layout = new QVBoxLayout(parameterGroup);

			QWidget *parameterWidget;

			if (it->Type == "String")
			{
				parameterWidget = new QLineEdit(parameterGroup);
				parameterWidget->installEventFilter(&selector);
			}
			else
			{
				const beCore::ComponentReflector *pReflector = beCore::GetComponentTypes().GetReflector(it->Type);

				if (pReflector)
				{
					ComponentSelectorWidget *csw = new ComponentSelectorWidget(pReflector, nullptr, pEditor, parameterGroup);
					csw->installFocusHandler(&selector);
					parameterWidget = csw;
				}
				else
					parameterWidget = new QLabel(
							ComponentSelectorWidget::tr("Unknown type '%1'").arg( toQt(it->Type) ),
							parameterGroup
						);
			}

			layout->addWidget(parameterWidget);

			if (!firstParameterWidget)
				firstParameterWidget = parameterWidget;

			parameterGroup->setLayout(layout);
			ui.newLayout->addWidget(parameterGroup);
		}

		ui.newWrapper->setFocusProxy(firstParameterWidget);

		if (optionCount == 0)
		{
			ui.newButton->setChecked(true);
			selector.setFocusProxy(ui.newWrapper);
		}
		++optionCount;
	}
	// Remove new option, if not available
	else
	{
		hideAndRemove(*ui.newButton);
		hideAndRemove(*ui.newWrapper);
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

/// Makes a component selector setting string.
QString makeSettingString(const beCore::ComponentReflector &reflector, const QString &setting)
{
	return QString("componentSelectorWidget/%1/%2").arg(toQt(reflector.GetType())).arg(setting);
}

// Initialize the UI from current element or from the stored configuration.
void initUI(ComponentSelectorWidget &selector, Ui::ComponentSelectorWidget &ui, const beCore::ComponentReflector &reflector, const lean::any *pCurrent, Editor &editor)
{
	bool bFileInitialized = false;
	bool bNameInitialized = false;

	if (pCurrent && (reflector.HasName() || reflector.CanBeLoaded()))
	{
		beCore::ComponentState::T state = beCore::ComponentState::Unknown;
		QString name = toQt(reflector.GetNameOrFile(*pCurrent, &state));

		if (state == beCore::ComponentState::Filed)
		{
			ui.browseWidget->setPath(name);
			bFileInitialized = true;

			ui.fileButton->setChecked(true);
			selector.setFocusProxy(ui.browseWidget);
		}
		else if (state == beCore::ComponentState::Named)
		{
			ui.nameEdit->setText(name);
			bNameInitialized = true;

			ui.nameButton->setChecked(true);
			selector.setFocusProxy(ui.nameEdit);
		}
	}

	if (!bFileInitialized)
		ui.browseWidget->setPath( editor.settings()->value(makeSettingString(reflector, "file"), QDir::currentPath()).toString() );
	if (!bNameInitialized)
		ui.nameEdit->setText( editor.settings()->value(makeSettingString(reflector, "name"), QString()).toString() );
}

} // namespace

// Constructor.
ComponentSelectorWidget::ComponentSelectorWidget(const beCore::ComponentReflector *pReflector, const lean::any *pCurrent, Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pReflector( LEAN_ASSERT_NOT_NULL(pReflector) ),
	m_pCurrent( pCurrent )
{
	ui.setupUi(this);

	// Remove irrelevant controls & recursively build widget
	adaptUI(*this, ui, *m_pReflector, m_pEditor);

	// Initialize from current element or from stored configuration
	initUI(*this, ui, *m_pReflector, m_pCurrent, *m_pEditor);

	checkedConnect(ui.browseWidget, SIGNAL(browse()), this, SLOT(browse()));
}

// Destructor.
ComponentSelectorWidget::~ComponentSelectorWidget()
{
}

// Installs the given event handler on all relevant child widgets.
void ComponentSelectorWidget::installFocusHandler(QObject *handler)
{
	m_focusHandlers.push_back( LEAN_ASSERT_NOT_NULL(handler) );
}

// Browser requested.
void ComponentSelectorWidget::browse()
{
	QString location = ui.browseWidget->path();

	if (location.isEmpty())
		location = QDir::currentPath();

	// Open either breeze mesh or importable 3rd-party mesh format
	QString file = QFileDialog::getOpenFileName( this,
			tr("Select a ''%1' resource").arg( toQt(m_pReflector->GetType()) ),
			location,
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
	{
		QString path = ui.browseWidget->path();

		if (!path.isEmpty())
			m_pEditor->settings()->setValue(makeSettingString(*m_pReflector, "file"), path);

		return *m_pReflector->GetComponent( toUtf8Range(path), parameters );
	}
	else if (ui.nameButton->isChecked())
	{
		QString name = ui.nameEdit->text();

		if (!name.isEmpty())
			m_pEditor->settings()->setValue(makeSettingString(*m_pReflector, "name"), name);

		return *m_pReflector->GetComponentByName( toUtf8Range(name), parameters );
	}
	else
	{
		beCore::Parameters creationParameters;

		int parameterCount = ui.newLayout->count();

		for (int idx = 0; idx < parameterCount; ++idx)
		{
			QGroupBox *pGroupBox = qobject_cast<QGroupBox*>( ui.newLayout->itemAt(idx)->widget() );

			if (pGroupBox)
			{
				lean::utf8_string parameterName = toUtf8( pGroupBox->title() );
				QWidget &parameterWidget = *LEAN_ASSERT_NOT_NULL( pGroupBox->layout()->itemAt(0)->widget() );
				
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
		}

		return *m_pReflector->CreateComponent( creationParameters, parameters, ui.cloneCheckBox->isChecked() ? m_pCurrent : nullptr );
	}
}

// Filters focus events.
bool ComponentSelectorWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FocusIn)
	{
		QWidget *widget = static_cast<QWidget*>(obj);

		if (isChild(widget, ui.nameEdit))
			ui.nameButton->setChecked(true);
		else if (isChild(widget, ui.browseWidget))
			ui.fileButton->setChecked(true);
		else if (isChild(widget, ui.newWrapper))
			ui.newButton->setChecked(true);

		Q_FOREACH(QObject *handler, m_focusHandlers)
			if (handler->eventFilter(obj, event))
				return true;
	}

	return QObject::eventFilter(obj, event);
}
