#ifndef CREATIONPARAMETERWIDGETFACTORY_H
#define CREATIONPARAMETERWIDGETFACTORY_H

#include "CreationParameterWidget.h"

/// Creation parameter widget factory class.
class CreationParameterWidgetFactory
{
protected:
	CreationParameterWidgetFactory(const CreationParameterWidgetFactory&) { }
	CreationParameterWidgetFactory& operator =(const CreationParameterWidgetFactory&) { return *this; }

public:
	CreationParameterWidgetFactory() { }
	virtual ~CreationParameterWidgetFactory() { }

	/// Creates a creation parameter widget.
	virtual CreationParameterWidget* createWidget(const QString &parameterName, Editor *pEditor, QWidget *pParent = nullptr) const = 0;
};

template <class WidgetFactory>
class WidgetFactoryManager;

/// Gets the creation widget factory manager.
WidgetFactoryManager<CreationParameterWidgetFactory>& getCreationParameterWidgetFactory();

#endif
