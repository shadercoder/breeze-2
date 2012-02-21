/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_TEXTURE
#define BE_GRAPHICS_TEXTURE

#include "beGraphics.h"
#include "beFormat.h"
#include <beCore/beShared.h>
#include <beGraphics/beDevice.h>
#include <beCore/beOpaqueHandle.h>
#include <lean/smart/resource_ptr.h>

namespace beGraphics
{

/// Texture description.
struct TextureDesc
{
	uint4 Width;			///< Width in pixels.
	uint4 Height;			///< Height in pixels.
	uint4 Depth;			///< Depth in pixels.
	Format::T Format;		///< Pixel format.
	uint4 MipLevels;		///< Number of mip levels.

	/// Constructor.
	explicit TextureDesc(uint4 width = 0,
		uint4 height = 0,
		uint4 depth = 0,
		Format::T format = Format::Unknown,
		uint4 mipLevels = 0)
			: Width(width),
			Height(height),
			Depth(depth),
			Format(format),
			MipLevels(mipLevels) { }
};

/// Texture type enumeration.
namespace TextureType
{
	/// Enumeration.
	enum T
	{
		Texture1D,		///< 1D texture.
		Texture2D,		///< 2D texture.
		Texture3D,		///< 3D texture.

		NotATexture		///< Not a texture.
	};
}

class TextureCache;

/// Texture resource interface.
class Texture : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE Texture& operator =(const Texture&) { return *this; }

public:
	virtual ~Texture() throw() { };

	/// Gets the texture description.
	virtual TextureDesc GetDesc() const = 0;
	/// Gets the texture type.
	virtual TextureType::T GetType() const = 0;

	/// Gets the texture cache.
	virtual TextureCache* GetCache() const = 0;
};

/// Texture view interface.
class TextureView : public beCore::OptionalResource, public Implementation
{
protected:
	LEAN_INLINE TextureView& operator =(const TextureView&) { return *this; }

public:
	virtual ~TextureView() throw() { };

	/// Gets the texture description.
	virtual TextureDesc GetDesc() const = 0;
	/// Gets the texture type.
	virtual TextureType::T GetType() const = 0;

	/// Gets the texture cache.
	virtual TextureCache* GetCache() const = 0;
};

/// Texture view handle.
typedef beCore::OpaqueHandle<TextureView> TextureViewHandle;
using beCore::ToImpl;


// Prototypes
class SwapChain;

/// Loads a texture from the given file.
BE_GRAPHICS_API lean::resource_ptr<Texture, true> LoadTexture(const lean::utf8_ntri &fileName, const TextureDesc *pDesc, const Device &device, TextureCache *pCache = nullptr);
/// Loads a texture from the given memory.
BE_GRAPHICS_API lean::resource_ptr<Texture, true> LoadTexture(const char *data, uint4 dataLength, const TextureDesc *pDesc, const Device &device, TextureCache *pCache = nullptr);
/// Creates a texture view from the given texture.
BE_GRAPHICS_API lean::resource_ptr<TextureView, true> ViewTexture(const Texture &texture, const Device &device);

/// Gets the back buffer.
BE_GRAPHICS_API lean::resource_ptr<Texture, true> GetBackBuffer(const SwapChain &swapChain, uint4 index = 0);

} // namespace

#endif