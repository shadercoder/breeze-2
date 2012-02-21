#ifndef WIDGETFACTORYMANAGER_H
#define WIDGETFACTORYMANAGER_H

#include <QtCore/QMap>

/// Widget factory class.
template <class WidgetFactory>
class WidgetFactoryManager
{
private:
	typedef QMap<QString, const WidgetFactory*> factory_map;
	factory_map m_factories;

public:
	/// Adds a factory.
	void addFactory(const QString &name, const WidgetFactory *pFactory)
	{
		m_factories[name] = pFactory;
	}
	/// Removes a factory.
	void removeFactory(const QString &name)
	{
		m_factories[name] = nullptr;
	}

	/// Gets a factory.
	const WidgetFactory* getFactory(const QString &name) const
	{
		return m_factories[name];
	}
};

#endif
