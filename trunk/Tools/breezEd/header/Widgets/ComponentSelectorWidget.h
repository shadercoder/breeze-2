#ifndef COMPONENTSELECTORWIDGET_H
#define COMPONENTSELECTORWIDGET_H

#include "ui_ComponentSelectorWidget.h"

#include <beCore/beComponentReflector.h>

#include <lean/containers/any.h>
#include <lean/smart/cloneable_obj.h>

#include <QtCore/QList>

class Editor;
class SceneDocument;

/// Component selector tool.
class ComponentSelectorWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::ComponentSelectorWidget ui;

	Editor *m_pEditor;

	const beCore::ComponentReflector *m_pReflector;

	typedef QList<QObject*> object_list;
	object_list m_focusHandlers;

protected:
	/// Filters focus events.
	virtual bool eventFilter(QObject *obj, QEvent *event);

public:
	/// Constructor.
	ComponentSelectorWidget(const beCore::ComponentReflector *pReflector, const lean::any *pCurrent, Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~ComponentSelectorWidget();

	/// Gets the selected component.
	lean::cloneable_obj<lean::any> acquireComponent(SceneDocument &document) const;

	/// Installs the given event handler on all relevant child widgets.
	void installFocusHandler(QObject *handler);

public Q_SLOTS:
	/// Browser requested.
	void browse();
};

#endif
