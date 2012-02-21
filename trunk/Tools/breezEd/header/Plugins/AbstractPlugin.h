#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

/// Plugin base class.
template <class Parameter>
class AbstractPlugin
{
protected:
	AbstractPlugin(const AbstractPlugin&) { }
	AbstractPlugin& operator =(const AbstractPlugin&) { return *this; }

public:
	AbstractPlugin() { }
	virtual ~AbstractPlugin() { }

	/// Initializes the plugin.
	virtual void initialize(Parameter parameter) const = 0;
	/// Finalizes the plugin.
	virtual void finalize(Parameter parameter) const = 0;
};

#endif
