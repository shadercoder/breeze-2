#include "stdafx.h"
#include "Widgets/ComponentPickerFactory.h"

#include "Plugins/FactoryManager.h"

/// Gets the component picker factory manager.
FactoryManager<ComponentPickerFactory>& getComponentPickerFactories()
{
	static FactoryManager<ComponentPickerFactory> manager;
	return manager;
}
