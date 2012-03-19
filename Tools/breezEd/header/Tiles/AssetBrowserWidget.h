#ifndef ASSETBROWSERWIDGET_H
#define ASSETBROWSERWIDGET_H

#include "ui_AssetBrowserWidget.h"

#include <lean/smart/scoped_ptr.h>

class Editor;
class AbstractDocument;
class SceneDocument;

/// Entity builder tool.
class AssetBrowserWidget : public QWidget
{
	Q_OBJECT

public:
	class CreationInteraction;

private:
	Ui::AssetBrowserWidget ui;

	Editor *m_pEditor;

	SceneDocument *m_pDocument;

	lean::scoped_ptr<CreationInteraction> m_pInteraction;

public:
	/// Constructor.
	AssetBrowserWidget(Editor *pEditor, QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~AssetBrowserWidget();

public Q_SLOTS:
	// Sets the current document.
	void setDocument(AbstractDocument *pDocument);

	/// Handles asset dragging.
	void dragStarted(QTreeWidgetItem *pItem);
	/// Handles asset dragging.
	void dragFinished(QTreeWidgetItem *pItem);
};

#endif
