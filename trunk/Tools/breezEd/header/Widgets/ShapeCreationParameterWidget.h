#ifndef SHAPECREATIONPARAMETERWIDGET_H
#define SHAPECREATIONPARAMETERWIDGET_H

#include "FileCreationParameterWidget.h"

/// Shape creation parameter widget.
class ShapeCreationParameterWidget : public FileCreationParameterWidget
{
	Q_OBJECT

protected:
	/// Browses for a file.
	virtual QString onBrowse(const QString &path);

public:
	/// Constructor.
	ShapeCreationParameterWidget(const QString &name, Editor *pEditor, QWidget *pParent = nullptr);
	/// Destructor.
	~ShapeCreationParameterWidget();
};

#endif
