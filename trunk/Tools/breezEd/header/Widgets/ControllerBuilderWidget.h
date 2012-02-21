#ifndef CONTROLLERBUILDERWIDGET_H
#define CONTROLLERBUILDERWIDGET_H

#include "ui_ControllerBuilderWidget.h"

#include <beEntitySystem/beControllerSerializer.h>

class Editor;
class CreationParameterWidget;
class SceneDocument;

/// Controller builder tool.
class ControllerBuilderWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::ControllerBuilderWidget ui;

	Editor *m_pEditor;

	const beEntitySystem::ControllerSerializer *m_pSerializer;

	QList<CreationParameterWidget*> m_parameters;

public:
	/// Constructor.
	ControllerBuilderWidget(const beEntitySystem::ControllerSerializer *pSerializer, Editor *pEditor, QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~ControllerBuilderWidget();

	/// Sets the creation parameter in the given set.
	virtual void setParameters(beCore::Parameters &parameters, SceneDocument &document) const;

	/// Gets the serializer.
	const beEntitySystem::ControllerSerializer* serializer() const { return m_pSerializer; }

Q_SIGNALS:
	/// Move the given controller up.
	void moveUp();
	/// Move the given controller down.
	void moveDown();
};

#endif
