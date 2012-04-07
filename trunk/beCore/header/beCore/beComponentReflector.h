/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_COMPONENT_REFLECTOR
#define BE_CORE_COMPONENT_REFLECTOR

#include "beCore.h"
#include <lean/tags/noncopyable.h>

#include <beCore/beParameters.h>
#include <beCore/beParameterSet.h>

#include <lean/containers/any.h>
#include <lean/smart/cloneable_obj.h>

#include "beExchangeContainers.h"

namespace beCore
{

/// Specific parameter.
struct ComponentParameter
{
	utf8_ntr Name;	///< Parameter name.
	utf8_ntr Type;	///< Parameter type.

	/// Constructor
	ComponentParameter(const utf8_ntr &name, const utf8_ntr &type)
		: Name(name),
		Type(type) { }
};

/// Component parameter range.
typedef lean::range<const ComponentParameter*> ComponentParameters;

/// Component state enumeration.
namespace ComponentState
{
	// Enum.
	enum T
	{
		NotSet,		/// Null.
		Unknown,	/// Valid, but unmanaged.
		Named,		/// Valid & named.
		Filed		/// Valid & filed.
	};
}

/// Provides generic access to abstract component types.
class ComponentReflector
{
protected:
	ComponentReflector& operator =(const ComponentReflector&) { return *this; }
	~ComponentReflector() throw() { }

public:
	/// Returns true, if the component can be created.
	virtual bool CanBeCreated() const { return false; }
	/// Gets a list of creation parameters.
	virtual ComponentParameters GetCreationParameters() const { return ComponentParameters(); };
	/// Creates a component from the given parameters.
	virtual lean::cloneable_obj<lean::any, true> CreateComponent(
		const beCore::Parameters &creationParameters, const beCore::ParameterSet &parameters,
		const lean::any *pPrototype = nullptr) const
	{
		return nullptr;
	}

	/// Returns true, if the component can be named.
	virtual bool HasName() const { return CanBeNamed(); }
	/// Returns true, if the component can be named.
	virtual bool CanBeNamed() const { return false; }
	/// Gets a component by name.
	virtual lean::cloneable_obj<lean::any, true> GetComponentByName(const utf8_ntri &name, const beCore::ParameterSet &parameters) const { return nullptr; }

	/// Returns true, if the component can be loaded from a file.
	virtual bool CanBeLoaded() const { return false; }
	/// Gets a fitting file extension, if available.
	virtual utf8_ntr GetFileExtension() const { return utf8_ntr(""); }
	/// Gets a component from the given file.
	virtual lean::cloneable_obj<lean::any, true> GetComponent(const utf8_ntri &file, const beCore::ParameterSet &parameters) const { return nullptr; }

	/// Gets the name or file of the given component.
	virtual beCore::Exchange::utf8_string GetNameOrFile(const lean::any &component, ComponentState::T *pState = nullptr) const { return beCore::Exchange::utf8_string(); }

	/// Gets the component type reflected.
	virtual utf8_ntr GetType() const = 0;
};

} // namespace

#endif