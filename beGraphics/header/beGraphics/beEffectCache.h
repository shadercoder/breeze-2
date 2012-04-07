/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_EFFECT_CACHE
#define BE_GRAPHICS_EFFECT_CACHE

#include "beGraphics.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include "beEffect.h"
#include <lean/smart/resource_ptr.h>
#include <beCore/beExchangeContainers.h>
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>
#include <beCore/beDependencies.h>

namespace beGraphics
{

namespace Exchange = beCore::Exchange;

/// Effect macro.
struct EffectMacro
{
	lean::utf8_ntr Name;		///< Macro name.
	lean::utf8_ntr Definition;	///< Macro definition.

	/// Constructor.
	explicit EffectMacro(const lean::utf8_ntr &name, const lean::utf8_ntr &definition)
		: Name(name),
		Definition(definition) { }
};

/// Effect cache.
class EffectCache : public lean::noncopyable, public beCore::Resource, public Implementation
{
protected:
	LEAN_INLINE EffectCache& operator =(const EffectCache&) { return *this; }

public:
	virtual ~EffectCache() throw() { }

	/// Gets the given effect compiled using the given options from file.
	virtual Effect* GetEffect(const lean::utf8_ntri &file, const EffectMacro *pMacros, size_t macroCount) = 0;
	/// Gets the given effect compiled using the given options from file.
	virtual Effect* GetEffect(const lean::utf8_ntri &file, const utf8_ntri &macros) = 0;

	/// Gets the given effect compiled using the given options from file, if it has been loaded.
	virtual Effect* IdentifyEffect(const lean::utf8_ntri &file, const utf8_ntri &macros) const = 0;

	/// Gets the file (or name) of the given effect.
	virtual utf8_ntr GetFile(const Effect &effect, beCore::Exchange::utf8_string *pMacros = nullptr, bool *pIsFile = nullptr) const = 0;

	/// Checks if the given effects are cache-equivalent.
	virtual bool Equivalent(const Effect &left, const Effect &right, bool bIgnoreMacros = false) const = 0;

	/// Notifies dependent listeners about dependency changes.
	virtual void NotifyDependents() = 0;
	/// Gets the dependencies registered for the given effect.
	virtual beCore::Dependency<Effect*>* GetDependency(const Effect &effect) = 0;

	/// Gets the path resolver.
	virtual const beCore::PathResolver& GetPathResolver() const = 0;
};

/// Mangles the given file name & macros.
BE_GRAPHICS_API Exchange::utf8_string MangleFilename(const lean::utf8_ntri &file, const EffectMacro *pMacros, size_t macroCount);
/// Mangles the given file name & macros.
BE_GRAPHICS_API Exchange::utf8_string MangleFilename(const lean::utf8_ntri &file, const lean::utf8_ntri &macros);

// Prototypes
class Device;

/// Creates a new effect cache.
BE_GRAPHICS_API lean::resource_ptr<EffectCache, true> CreateEffectCache(const Device &device, const utf8_ntri &cacheDir,
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

}

#endif