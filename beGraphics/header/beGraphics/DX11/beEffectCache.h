/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_EFFECT_CACHE_DX11
#define BE_GRAPHICS_EFFECT_CACHE_DX11

#include "beGraphics.h"
#include "../beEffectCache.h"
#include "beEffect.h"
#include <D3D11.h>
#include <lean/pimpl/pimpl_ptr.h>
#include <lean/smart/resource_ptr.h>

namespace beGraphics
{

namespace DX11
{

/// Effect cache implementation.
class EffectCache : public beGraphics::EffectCache
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_GRAPHICS_DX11_API EffectCache(ID3D11Device *pDevice, const utf8_ntri &cacheDir, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_GRAPHICS_DX11_API ~EffectCache();

	/// Gets the given effect compiled using the given options from file.
	BE_GRAPHICS_DX11_API Effect* GetEffect(const lean::utf8_ntri &file, const EffectMacro *pMacros, size_t macroCount);
	/// Gets the given effect compiled using the given options from file.
	BE_GRAPHICS_DX11_API Effect* GetEffect(const lean::utf8_ntri &file, const utf8_ntri &macros);

	/// Gets the given effect compiled using the given options from file, if it has been loaded.
	BE_GRAPHICS_DX11_API Effect* IdentifyEffect(const lean::utf8_ntri &file, const utf8_ntri &macros) const;

	/// Gets the file (or name) of the given effect.
	BE_GRAPHICS_DX11_API utf8_ntr GetFile(const beGraphics::Effect &effect, beCore::Exchange::utf8_string *pMacros = nullptr, bool *pIsFile = nullptr) const;

	/// Checks if the given effects are cache-equivalent.
	BE_GRAPHICS_DX11_API bool Equivalent(const beGraphics::Effect &left, const beGraphics::Effect &right, bool bIgnoreMacros = false) const;

	/// Notifies dependent listeners about dependency changes.
	BE_GRAPHICS_DX11_API void NotifyDependents();
	/// Gets the dependencies registered for the given effect.
	BE_GRAPHICS_DX11_API beCore::Dependency<beGraphics::Effect*>* GetDependency(const beGraphics::Effect &effect);

	/// Gets the path resolver.
	BE_GRAPHICS_DX11_API const beCore::PathResolver& GetPathResolver() const;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return DX11Implementation; }
};

template <> struct ToImplementationDX11<beGraphics::EffectCache> { typedef EffectCache Type; };

} // namespace

} // namespace

#endif