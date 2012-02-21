#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "AbstractPlugin.h"
#include <list>

/// Plugin manager class.
template <class Parameter>
class PluginManager
{
private:
	struct Plugin
	{
		AbstractPlugin<Parameter> *pPlugin;
		bool bInitialized;

		Plugin(AbstractPlugin<Parameter> *pPlugin, bool bInitialized = false)
			: pPlugin(pPlugin),
			bInitialized(bInitialized) { }
	};

	typedef std::list<Plugin> plugin_list;
	plugin_list m_plugins;

public:
	/// Adds a plugin.
	void addPlugin(AbstractPlugin<Parameter> *pPlugin)
	{
		m_plugins.push_back(pPlugin);
	}
	/// Removes a plugin.
	void removePlugin(AbstractPlugin<Parameter> *pPlugin)
	{
		for (plugin_list::iterator it = m_plugins.begin();
			it != m_plugins.end(); )
			if (it->pPlugin == pPlugin)
				it = m_plugins.erase(it);
			else
				++it;
	}

	/// Initializes all plugins.
	void initializePlugins(Parameter parameter)
	{
		for (plugin_list::iterator it = m_plugins.begin();
			it != m_plugins.end(); ++it)
			if (!it->bInitialized)
			{
				it->pPlugin->initialize(parameter);
				it->bInitialized = true;
			}
	}
	/// Finalizes all plugins.
	void finalizePlugins(Parameter parameter)
	{
		for (plugin_list::iterator it = m_plugins.begin();
			it != m_plugins.end(); ++it)
			if (it->bInitialized)
			{
				it->bInitialized = false;
				it->pPlugin->finalize(parameter);
			}
	}
};

#endif
