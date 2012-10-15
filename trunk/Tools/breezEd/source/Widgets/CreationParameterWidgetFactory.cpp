#include "stdafx.h"
#include "Widgets/CreationParameterWidgetFactory.h"

#include "Plugins/FactoryManager.h"

/// Gets the creation widget factory manager.
FactoryManager<CreationParameterWidgetFactory>& getCreationParameterWidgetFactory()
{
	static FactoryManager<CreationParameterWidgetFactory> manager;
	return manager;
}
