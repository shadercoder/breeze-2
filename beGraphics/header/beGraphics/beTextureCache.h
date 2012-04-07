/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_TEXTURE_CACHE
#define BE_GRAPHICS_TEXTURE_CACHE

#include "beGraphics.h"
#include <beCore/beShared.h>
#include <lean/tags/noncopyable.h>
#include "beTexture.h"
#include <lean/smart/resource_ptr.h>
#include <beCore/bePathResolver.h>
#include <beCore/beContentProvider.h>
#include <beCore/beDependencies.h>

namespace beGraphics
{

/// Texture cache.
class TextureCache : public lean::noncopyable, public beCore::Resource, public Implementation
{
protected:
	LEAN_INLINE TextureCache& operator =(const TextureCache&) { return *this; }

public:
	virtual ~TextureCache() throw() { }

	/// Gets a texture from the given file.
	virtual Texture* GetTexture(const lean::utf8_ntri &file) = 0;
	/// Gets a texture view for the given texture.
	virtual TextureView* GetTextureView(const Texture &texture) = 0;

	/// Gets a texture view from the given file.
	LEAN_INLINE TextureView* GetTextureView(const lean::utf8_ntri &file)
	{
		return GetTextureView( *GetTexture(file) ); 
	}

	/// Gets the file (or name) of the given texture.
	virtual utf8_ntr GetFile(const beGraphics::Texture &texture, bool *pIsFile = nullptr) const = 0;
	/// Gets the file (or name) of the given texture.
	virtual utf8_ntr GetFile(const beGraphics::TextureView &texture, bool *pIsFile = nullptr) const = 0;

	/// Notifies dependent listeners about dependency changes.
	virtual void NotifyDependents() = 0;
	/// Gets the dependencies registered for the given texture.
	virtual beCore::Dependency<Texture*>* GetDependencies(const beGraphics::Texture &texture) = 0;
	/// Gets the dependencies registered for the given texture.
	virtual beCore::Dependency<Texture*>* GetDependencies(const beGraphics::TextureView &texture) = 0;

	/// Gets the path resolver.
	virtual const beCore::PathResolver& GetPathResolver() const = 0;
};

// Prototypes
class Device;

/// Creates a new texture cache.
BE_GRAPHICS_API lean::resource_ptr<TextureCache, true> CreateTextureCache(const Device &device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider);

} // namespace

#endif