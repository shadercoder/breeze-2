#include "stdafx.h"
#include "Widgets/CreationParameterWidgetFactory.h"

#include "Plugins/WidgetFactoryManager.h"

/// Gets the creation widget factory manager.
WidgetFactoryManager<CreationParameterWidgetFactory>& getCreationParameterWidgetFactory()
{
	static WidgetFactoryManager<CreationParameterWidgetFactory> manager;
	return manager;
}
