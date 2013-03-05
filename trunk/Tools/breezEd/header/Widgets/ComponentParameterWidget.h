#ifndef COMPONENTPARAMETERWIDGET_H
#define COMPONENTPARAMETERWIDGET_H

#include "Utility/CollectionListWidget.h"

#include <beCore/beParameters.h>
#include <beCore/beComponentReflector.h>

class Editor;

/// Entity builder tool.
class ComponentParameterWidget : public QWidget
{
	Q_OBJECT

public:
	/// Constructor.
	ComponentParameterWidget(QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Constructor.
	ComponentParameterWidget(const beCore::ComponentParameters &parameters, const beCore::Parameters &defaultValues, bool bHasPrototype, Editor *editor,
		QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~ComponentParameterWidget();

	/// Gets the selected parameter values.
	void getParameters(beCore::Parameters &parameters) const;
	/// Gets the selected parameter values.
	beCore::Parameters getParameters() const;
	/// Checks if the widget offers any parameters.
	bool hasParameters() const;
};

#endif
