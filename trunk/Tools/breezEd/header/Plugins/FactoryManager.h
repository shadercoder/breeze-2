#ifndef FACTORYMANAGER_H
#define FACTORYMANAGER_H

#include <QtCore/QMap>

/// Factory manager class.
template <class Factory>
class FactoryManager
{
private:
	typedef QMap<QString, const Factory*> factory_map;
	factory_map m_factories;

	const Factory *m_pDefaultFactory;

public:
	/// Factory type.
	typedef Factory factory_type;

	/// Constructor.
	FactoryManager()
		: m_pDefaultFactory() { }

	/// Sets the default factory.
	void setDefaultFactory(const Factory *pFactory)
	{
		m_pDefaultFactory = pFactory;
	}
	/// Unsets the given default factory.
	void unsetDefaultFactory(const Factory *pFactory)
	{
		if (pFactory == m_pDefaultFactory)
			m_pDefaultFactory = nullptr;
	}

	/// Adds a factory.
	void addFactory(const QString &name, const factory_type *pFactory)
	{
		m_factories[name] = pFactory;
	}
	/// Removes a factory.
	void removeFactory(const QString &name)
	{
		m_factories[name] = nullptr;
	}

	/// Gets a factory.
	const factory_type* getFactory(const QString &name) const
	{
		factory_map::const_iterator it = m_factories.find(name);

		return (it != m_factories.end())
			? *it
			: m_pDefaultFactory;
	}
};

#endif
