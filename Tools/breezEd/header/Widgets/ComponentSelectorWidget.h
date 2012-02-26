#ifndef COMPONENTSELECTORWIDGET_H
#define COMPONENTSELECTORWIDGET_H

#include "ui_ComponentSelectorWidget.h"

#include <beCore/beComponentReflector.h>

#include <lean/containers/any.h>
#include <lean/smart/cloneable_obj.h>

class SceneDocument;

/// Component selector tool.
class ComponentSelectorWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::ComponentSelectorWidget ui;

	const beCore::ComponentReflector *m_pReflector;

public:
	/// Constructor.
	ComponentSelectorWidget(const beCore::ComponentReflector *pReflector, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~ComponentSelectorWidget();

	/// Gets the selected component.
	lean::cloneable_obj<lean::any> acquireComponent(SceneDocument &document) const;

public Q_SLOTS:
	/// Browser requested.
	void browse();
};

#endif
