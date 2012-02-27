#ifndef ENTITYBUILDERWIDGET_H
#define ENTITYBUILDERWIDGET_H

#include "ui_EntityBuilderWidget.h"

class Editor;
class AbstractDocument;
class SceneDocument;
class ControllerBuilderWidget;

/// Entity builder tool.
class EntityBuilderWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::EntityBuilderWidget ui;

	Editor *m_pEditor;

	SceneDocument *m_pDocument;

public:
	/// Constructor.
	EntityBuilderWidget(Editor *pEditor, QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~EntityBuilderWidget();

public Q_SLOTS:
	/// Sets the current document.
	void setDocument(AbstractDocument *pDocument);

	/// Creates an entity.
	void createEntity();

	/// Adds a controller of the given type.
	void addController(const QString &type);

	/// Moves the given controller up.
	void moveUp();
	/// Moves the given controller down.
	void moveDown();
};

#endif
