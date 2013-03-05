#ifndef ENTITYSELECTIONWIDGET_H
#define ENTITYSELECTIONWIDGET_H

#include "ui_EntitySelectionWidget.h"

#include <beEntitySystem/beEntities.h>

class QTimer;

class Editor;
class SceneDocument;
class AbstractDocument;

/// Entity selection tool.
class EntitySelectionWidget : public QWidget
{
	Q_OBJECT

public:
	/// Entity vector.
	typedef QVector<beEntitySystem::Entity*> entity_vector;

private:
	Ui::EntitySelectionWidget ui;

	Editor *m_pEditor;

	SceneDocument *m_pDocument;
	entity_vector m_selection;

public:
	/// Constructor.
	EntitySelectionWidget(Editor *pEditor, QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~EntitySelectionWidget();

public Q_SLOTS:
	/// Sets the undo stack of the given document.
	void setDocument(AbstractDocument *pDocument);
	/// Sets the active selection of the given document.
	void setActiveSelection(SceneDocument *pDocument);

	/// Updates the ui.
	void selectionChanged();

	/// Deselects currently selected entities.
	void deselect();
	/// Saves selected entities into an asset.
	void toAsset();
};

#endif
