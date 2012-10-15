#ifndef GENERICCOMPONENTPICKER_H
#define GENERICCOMPONENTPICKER_H

#include "ComponentPicker.h"

#include "ui_GenericComponentPicker.h"
#include "Utility/UI.h"

#include <beCore/beComponentReflector.h>

#include <lean/containers/any.h>
#include <lean/smart/cloneable_obj.h>

#include <QtCore/QList>

class Editor;
class SceneDocument;

/// Component selector tool.
class GenericComponentPicker : public ComponentPicker
{
	Q_OBJECT

private:
	UI<Ui::GenericComponentPicker> ui;

	Editor *m_pEditor;

	const beCore::ComponentReflector *m_pReflector;
	lean::cloneable_obj<lean::any, true> m_pCurrent;

protected:
	/// Filters focus events.
	virtual bool eventFilter(QObject *obj, QEvent *event);

public:
	/// Constructor.
	GenericComponentPicker(const beCore::ComponentReflector *pReflector, const lean::any *pCurrent, Editor *pEditor, QWidget *pParent = nullptr, Qt::WFlags flags = 0);
	/// Destructor.
	~GenericComponentPicker();

	/// Gets the selected component.
	lean::cloneable_obj<lean::any> acquireComponent(SceneDocument &document) const;

public Q_SLOTS:
	/// Browser requested.
	void browse();
};

#endif
