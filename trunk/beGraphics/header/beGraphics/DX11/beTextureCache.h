/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_TEXTURE_CACHE_DX11
#define BE_GRAPHICS_TEXTURE_CACHE_DX11

#include "beGraphics.h"
#include "../beTextureCache.h"
#include <D3D11.h>
#include <lean/pimpl/pimpl_ptr.h>
#include <lean/smart/resource_ptr.h>

namespace beGraphics
{

namespace DX11
{

/// Texture cache implementation.
class TextureCache : public beGraphics::TextureCache
{
public:
	struct M;

private:
	lean::pimpl_ptr<M> m;

public:
	/// Constructor.
	BE_GRAPHICS_DX11_API TextureCache(ID3D11Device *pDevice, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);
	/// Destructor.
	BE_GRAPHICS_DX11_API ~TextureCache();

	/// Gets a texture from the given file.
	BE_GRAPHICS_DX11_API beGraphics::Texture* GetTexture(const lean::utf8_ntri &file, bool bSRGB = false);
	/// Gets a texture view for the given texture.
	BE_GRAPHICS_DX11_API beGraphics::TextureView* GetTextureView(const beGraphics::Texture &texture);

	/// Gets the file (or name) of the given texture.
	BE_GRAPHICS_DX11_API utf8_ntr GetFile(const Texture &texture, bool *pIsFile = nullptr) const;
	/// Gets the file (or name) of the given texture.
	BE_GRAPHICS_DX11_API utf8_ntr GetFile(const TextureView &texture, bool *pIsFile = nullptr) const;

	/// Notifies dependent listeners about dependency changes.
	BE_GRAPHICS_DX11_API void NotifyDependents();
	/// Gets the dependencies registered for the given texture.
	BE_GRAPHICS_DX11_API beCore::Dependency<beGraphics::Texture*>* GetDependencies(const beGraphics::Texture &texture);
	/// Gets the dependencies registered for the given texture.
	BE_GRAPHICS_DX11_API beCore::Dependency<beGraphics::Texture*>* GetDependencies(const beGraphics::TextureView &texture);

	/// Gets the path resolver.
	BE_GRAPHICS_DX11_API const beCore::PathResolver& GetPathResolver() const;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return DX11Implementation; }
};

template <> struct ToImplementationDX11<beGraphics::TextureCache> { typedef TextureCache Type; };

} // namespace

} // namespace

#endif