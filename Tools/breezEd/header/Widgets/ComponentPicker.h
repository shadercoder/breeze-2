#ifndef COMPONENTPICKER_H
#define COMPONENTPICKER_H

#include <QtWidgets/QWidget>

#include <beCore/beComponentReflector.h>

#include <lean/containers/any.h>
#include <lean/smart/cloneable_obj.h>

class SceneDocument;

/// Component picker tool.
class ComponentPicker : public QWidget
{
	Q_OBJECT

private:
	typedef QList<QObject*> object_list;
	object_list m_focusHandlers;

protected:
	/// Filters focus events.
	virtual bool eventFilter(QObject *obj, QEvent *event);

public:
	/// Constructor.
	ComponentPicker(QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~ComponentPicker();

	/// Gets the selected component.
	virtual lean::cloneable_obj<lean::any> acquireComponent(SceneDocument &document) const = 0;

	/// Installs the given event handler on all relevant child widgets.
	virtual void installFocusHandler(QObject *handler);
};

#endif
