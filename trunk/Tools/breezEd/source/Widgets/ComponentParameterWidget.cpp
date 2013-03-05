#include "stdafx.h"
#include "Widgets/ComponentParameterWidget.h"
#include "Tiles/ComponentBrowserWidget.h"

#include <beCore/beParameterSet.h>
#include <beCore/beComponentReflector.h>
#include <beCore/beReflectionTypes.h>

#include "Utility/CollectionListWidget.h"
#include "Widgets/FileCollectionWidget.h"

#include "Utility/WidgetSupport.h"
#include "Utility/Strings.h"
#include "Utility/Checked.h"

#include <lean/logging/errors.h>

namespace
{

typedef std::vector<bec::reflection_type<bec::ReflectionType::String>::type> ExchangeStringVector;

// Gets the strings.
ExchangeStringVector getStrings(const CollectionListWidget &list)
{
	ExchangeStringVector components;
	QList<QStandardItem*> items = list.items();
	components.reserve(items.size());

	Q_FOREACH (QStandardItem *item, items)
		components.push_back(
				toUtf8Range(item->text()).get().to<ExchangeStringVector::value_type>()
			);

	return components;
}

void adaptUI(QWidget *widget, const beCore::ComponentParameters &parameters, const beCore::Parameters &defaultValues, bool bHasPrototype, Editor *editor)
{
	for (beCore::ComponentParameters::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		const lean::any *pDefaultValue = defaultValues.GetAnyValue(defaultValues.GetID(it->Name));

		QGroupBox *parameterGroup = new QGroupBox( toQt(it->Name), widget );
		parameterGroup->setFlat(true);
		QVBoxLayout *layout = new QVBoxLayout(parameterGroup);
		layout->setContentsMargins(0, 1, 0, 1);

		if (bHasPrototype && (it->Flags & bec::ComponentParameterFlags::Deducible) || it->Flags & bec::ComponentParameterFlags::Optional)
		{
			parameterGroup->setCheckable(true);
			parameterGroup->setChecked(pDefaultValue != nullptr);
		}

		QWidget *parameterWidget;

		// Strings
		if (bec::GetReflectionType(it->Type) == bec::ReflectionType::String)
		{
			if (~it->Flags & bec::ComponentParameterFlags::Array)
				parameterWidget = new QLineEdit(parameterGroup);
			else
				parameterWidget = new CollectionListWidget(parameterGroup);
		}
		// Files
		else if (bec::GetReflectionType(it->Type) == bec::ReflectionType::File)
		{
			if (~it->Flags & bec::ComponentParameterFlags::Array)
				parameterWidget = new BrowseWidget(parameterGroup);
			else
				parameterWidget = new FileCollectionWidget(editor, parameterGroup);
		}
		// Booleans
		else if (bec::GetReflectionType(it->Type) == bec::ReflectionType::Boolean)
		{
			QCheckBox *checkBox = new QCheckBox(parameterGroup);
			new TrueFalseBinder(checkBox);
			parameterWidget = checkBox;
		}
		// Components
		else
		{
			const beCore::ComponentTypeDesc *pParameterTypeDesc = beCore::GetComponentTypes().GetDesc(it->Type);

			if (pParameterTypeDesc && pParameterTypeDesc->Reflector)
			{
				if (~it->Flags & bec::ComponentParameterFlags::Array)
				{
					ComponentSelectorWidget *selector = new ComponentSelectorWidget(pParameterTypeDesc, editor, parameterGroup);
					if (pDefaultValue)
						selector->setComponent(pDefaultValue, toQt(pParameterTypeDesc->Reflector->GetInfo(*pDefaultValue).Name)); // TODO: Handle emtpy name
					parameterWidget = selector;
				}
				else
				{
					ComponentCollectionWidget *list = new ComponentCollectionWidget(pParameterTypeDesc, editor, parameterGroup);
					if (pDefaultValue)
						for (uint4 configIdx = 0, configCount = (uint4) pDefaultValue->size(); configIdx < configCount; ++configIdx)
						{
							const lean::any *pComponent = lean::any_cast<lean::any>(pDefaultValue, configIdx);
							list->addComponent(pComponent, toQt(pParameterTypeDesc->Reflector->GetInfo(*pComponent).Name));
						}
					parameterWidget = list;
				}
			}
			else
				parameterWidget = new QLabel(
						ComponentBrowserWidget::tr("Unknown type '%1'").arg( toQt(it->Type->Name) ),
						parameterGroup
					);
				
		}

		layout->addWidget(parameterWidget);
		widget->layout()->addWidget(parameterGroup);
	}
}

void getParameters(const QWidget *widget, beCore::Parameters &parameters)
{
	for (int idx = 0, count = widget->layout()->count(); idx < count; ++idx)
	{
		QGroupBox *parameterGroup = qobject_cast<QGroupBox*>( widget->layout()->itemAt(idx)->widget() );

		// Ignore irrelevant widgets
		if (!parameterGroup || parameterGroup->isCheckable() && !parameterGroup->isChecked())
			continue;

		lean::utf8_string parameterName = toUtf8( parameterGroup->title() );
		QWidget &parameterWidget = *LEAN_ASSERT_NOT_NULL( parameterGroup->layout()->itemAt(0)->widget() );

		// Components
		if (ComponentSelectorWidget *selector = qobject_cast<ComponentSelectorWidget*>(&parameterWidget))
			parameters.SetAnyValue( parameters.Add(parameterName), selector->component() );
		else if (ComponentCollectionWidget *collection = qobject_cast<ComponentCollectionWidget*>(&parameterWidget))
			parameters.SetAnyValue(
					parameters.Add(parameterName),
					lean::any_vector< ComponentCollectionWidget::ComponentVector, lean::var_deref<lean::any> >(collection->components())
				);
		// Strings
		else if (QLineEdit *stringEdit = qobject_cast<QLineEdit*>(&parameterWidget))
			parameters.SetValue< bec::reflection_type<bec::ReflectionType::String>::type >(
					parameters.Add(parameterName),
					toUtf8Range(stringEdit->text()).get().to< bec::reflection_type<bec::ReflectionType::String>::type >()
				);
		else if (CollectionListWidget *collection = qobject_cast<CollectionListWidget*>(&parameterWidget))
			parameters.SetAnyValue(
					parameters.Add(parameterName),
					lean::any_vector< ExchangeStringVector >(getStrings(*collection))
				);
		// Booleans
		else if (QCheckBox *checkBox = qobject_cast<QCheckBox*>(&parameterWidget))
			parameters.SetValue< bec::reflection_type<bec::ReflectionType::Boolean>::type >(
					parameters.Add(parameterName),
					checkBox->isChecked()
				);
		else
			LEAN_ASSERT_UNREACHABLE();
	}
}

} // namespace

// Constructor.
ComponentParameterWidget::ComponentParameterWidget(QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget( new QLabel(tr("<no parameters>"), this) );
}

// Constructor.
ComponentParameterWidget::ComponentParameterWidget(const beCore::ComponentParameters &parameters, const beCore::Parameters &defaultValues,
												   bool bHasPrototype, Editor *editor,
												   QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags)
{
	(new QVBoxLayout(this))->setMargin(0);
	adaptUI(this, parameters, defaultValues, bHasPrototype, editor);
}

// Destructor.
ComponentParameterWidget::~ComponentParameterWidget()
{
}

// Gets the selected parameter values.
void ComponentParameterWidget::getParameters(beCore::Parameters &parameters) const
{
	::getParameters(this, parameters);
}

// Gets the selected parameter values.
beCore::Parameters ComponentParameterWidget::getParameters() const
{
	beCore::Parameters parameters;
	::getParameters(this, parameters);
	return parameters;
}

// Checks if the widget offers any parameters.
bool ComponentParameterWidget::hasParameters() const
{
	return this->layout()->count() > 0;
}
