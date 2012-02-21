#ifndef MESHCREATIONPARAMETERWIDGET_H
#define MESHCREATIONPARAMETERWIDGET_H

#include "FileCreationParameterWidget.h"

/// Mesh creation parameter widget.
class MeshCreationParameterWidget : public FileCreationParameterWidget
{
	Q_OBJECT

protected:
	/// Browses for a file.
	virtual QString onBrowse(const QString &path);

public:
	/// Constructor.
	MeshCreationParameterWidget(const QString &name, Editor *pEditor, QWidget *pParent = nullptr);
	/// Destructor.
	~MeshCreationParameterWidget();
};

#endif
