#ifndef ASSETBROWSERWIDGET_H
#define ASSETBROWSERWIDGET_H

#include "ui_AssetBrowserWidget.h"

class Editor;

/// Entity builder tool.
class AssetBrowserWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::AssetBrowserWidget ui;

	Editor *m_pEditor;

public:
	/// Constructor.
	AssetBrowserWidget(Editor *pEditor, QWidget *pParent = nullptr , Qt::WFlags flags = 0);
	/// Destructor.
	~AssetBrowserWidget();
};

#endif
