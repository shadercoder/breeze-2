#ifndef COMPONENTPICKERFACTORY_H
#define COMPONENTPICKERFACTORY_H

#include "ComponentPicker.h"

class Editor;

/// Component picker factory class.
class ComponentPickerFactory
{
protected:
	ComponentPickerFactory(const ComponentPickerFactory&) { }
	ComponentPickerFactory& operator =(const ComponentPickerFactory&) { return *this; }

public:
	ComponentPickerFactory() { }
	virtual ~ComponentPickerFactory() { }

	/// Creates a component picker.
	virtual ComponentPicker* createComponentPicker(const beCore::ComponentReflector *reflector, const lean::any *pCurrent,
		Editor *editor, QWidget *pParent = nullptr) const = 0;

	/// Browses for a component resource.
	virtual QString browseForComponent(const beCore::ComponentReflector &reflector, const QString &currentPath, Editor &editor, QWidget *pParent) const = 0;
};

template <class Factory>
class FactoryManager;

/// Gets the component picker factory manager.
FactoryManager<ComponentPickerFactory>& getComponentPickerFactories();

#endif
