#ifndef CREATIONPARAMETERWIDGET_H
#define CREATIONPARAMETERWIDGET_H

#include <QtCore/QObject>
#include <beCore/beParameters.h>

class Editor;
class SceneDocument;

/// Creation parameter widget base class.
class CreationParameterWidget : public QObject
{
protected:
	Editor *m_pEditor;

public:
	/// Constructor.
	CreationParameterWidget(Editor *pEditor, QObject *pParent = nullptr)
		: QObject( pParent ),
		m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ) { }

	/// Sets the creation parameter in the given set.
	virtual void setParameters(beCore::Parameters &parameters, SceneDocument &document) const = 0;

	/// Gets the widget.
	virtual QWidget* widget() = 0;
	/// Gets the widget.
	virtual const QWidget* widget() const = 0;
};

#endif
