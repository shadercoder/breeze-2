/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/DX11/beTextureCache.h"
#include "beGraphics/DX11/beTexture.h"
#include "beGraphics/DX11/beDevice.h"

#include <lean/smart/cloneable_obj.h>
#include <lean/smart/com_ptr.h>
#include <unordered_map>

#include <beCore/beFileWatch.h>
#include <beCore/beDependenciesImpl.h>

#include <lean/io/filesystem.h>

#include <lean/logging/log.h>

namespace beGraphics
{

namespace DX11
{

/// Texture cache implementation
struct TextureCache::M
{
	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	lean::com_ptr<ID3D11Device> pDevice;

	/// Texture
	struct ObservedTexture : public beCore::FileObserver
	{
		lean::resource_ptr<Texture> pTexture;
		lean::resource_ptr<TextureView> pTextureView;

		TextureCache *pCache;
		const utf8_string *file;
		bool bSRGB;

		beCore::Dependency<beGraphics::Texture*> *dependency;

		/// Constructor.
		ObservedTexture(lean::resource_ptr<Texture> pTexture, bool bSRGB, TextureCache *pCache)
			: pTexture(pTexture.transfer()),
			pCache(pCache),
			file(),
			bSRGB(bSRGB),
			dependency() { }

		/// Method called whenever an observed texture has changed.
		void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision);
	};

	typedef std::unordered_map<utf8_string, ObservedTexture> texture_map;
	texture_map textures;

	typedef std::unordered_map<ID3D11Resource*, ObservedTexture*> texture_info_map;
	texture_info_map textureInfo;

	beCore::FileWatch fileWatch;

	beCore::DependenciesImpl<beGraphics::Texture*> dependencies;

	/// Constructor.
	M(ID3D11Device *pDevice, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: pDevice(pDevice),
		resolver(resolver),
		provider(contentProvider)
	{
		LEAN_ASSERT(pDevice != nullptr);
	}
};

// Loads a texture from the given file.
lean::com_ptr<ID3D11Resource, true> LoadTexture(TextureCache::M &m, const lean::utf8_ntri &file, bool bSRGB)
{
	lean::com_ptr<beCore::Content> content = m.provider->GetContent(file);
	return DX11::LoadTexture(m.pDevice, content->Bytes(), static_cast<uint4>(content->Size()), nullptr, bSRGB);
}

// Constructor.
TextureCache::TextureCache(ID3D11Device *pDevice, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(pDevice, resolver, contentProvider) )
{
}

// Destructor.
TextureCache::~TextureCache()
{
}

// Gets a texture from the given file.
beGraphics::Texture* TextureCache::GetTexture(const lean::utf8_ntri &unresolvedFile, bool bSRGB)
{
	// Get absolute path
	beCore::Exchange::utf8_string excPath = m->resolver->Resolve(unresolvedFile, true);
	utf8_string path(excPath.begin(), excPath.end());

	// Try to find cached texture
	M::texture_map::iterator itTexture = m->textures.find(path);

	if (itTexture == m->textures.end())
	{
		// WARNING: Resource pointer invalid after transfer
		{
			LEAN_LOG("Attempting to load texture \"" << path << "\"");

			lean::resource_ptr<Texture> pTexture = ToImpl( CreateTexture( LoadTexture(*m, path, bSRGB).get(), this ).get() );

			LEAN_LOG("Texture \"" << unresolvedFile.c_str() << "\" created successfully");

			// Insert texture into cache
			itTexture = m->textures.insert(
					M::texture_map::value_type(
						path,
						M::ObservedTexture(
							pTexture.transfer(),
							bSRGB,
							this
						)
					)
				).first;
			itTexture->second.file = &itTexture->first;
		}
		
		// Link back to texture info
		m->textureInfo[itTexture->second.pTexture->GetResource()] = &itTexture->second;

		// Allow for monitoring of dependencies
		itTexture->second.dependency = m->dependencies.AddDependency(itTexture->second.pTexture);

		// Watch texture changes
		m->fileWatch.AddObserver(path, &itTexture->second);
	}

	return itTexture->second.pTexture;
}

// Gets a texture view for the given texture.
beGraphics::TextureView* TextureCache::GetTextureView(const beGraphics::Texture &texture)
{
	TextureView *pView = nullptr;

	ID3D11Resource *pResource = ToImpl(texture).GetResource();
	M::texture_info_map::const_iterator itInfo = m->textureInfo.find(pResource);

	if (itInfo != m->textureInfo.end())
	{
		if (!itInfo->second->pTextureView)
			itInfo->second->pTextureView = lean::new_resource<TextureView>(pResource, nullptr, m->pDevice, this);

		pView = itInfo->second->pTextureView;
	}

	return pView;
}

namespace
{

/// Gets the file (or name) of the given texture.
LEAN_INLINE utf8_ntr GetFile(const TextureCache::M &m, ID3D11Resource *pResource, bool *pIsFile)
{
	utf8_ntr file("");

	TextureCache::M::texture_info_map::const_iterator itInfo = m.textureInfo.find(pResource);

	if (itInfo != m.textureInfo.end())
	{
		file = utf8_ntr(*itInfo->second->file);

		if (pIsFile)
			// Back pointer only valid for files
			*pIsFile = (itInfo->second->pCache != nullptr);
	}

	return file;
}

} // namespace

// Gets the file (or name) of the given texture.
utf8_ntr TextureCache::GetFile(const beGraphics::Texture &texture, bool *pIsFile) const
{
	return DX11::GetFile(*m, ToImpl(texture).GetResource(), pIsFile);
}

// Gets the file (or name) of the given texture.
utf8_ntr TextureCache::GetFile(const beGraphics::TextureView &texture, bool *pIsFile) const
{
	return DX11::GetFile(*m, ToImpl(texture).GetResource(), pIsFile);
}

// Notifies dependent listeners about dependency changes.
void TextureCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

namespace
{

/// Gets the file (or name) of the given texture.
LEAN_INLINE beCore::Dependency<beGraphics::Texture*>* GetDependencies(TextureCache::M &m, ID3D11Resource *pResource)
{
	TextureCache::M::texture_info_map::iterator itTexture = m.textureInfo.find(pResource);

	return (itTexture != m.textureInfo.end())
		? itTexture->second->dependency
		: nullptr;
}

} // namespace

// Gets the dependencies registered for the given texture.
beCore::Dependency<beGraphics::Texture*>* TextureCache::GetDependencies(const beGraphics::Texture &texture)
{
	return DX11::GetDependencies( *m, ToImpl(texture).GetResource() );
}

// Gets the dependencies registered for the given texture.
beCore::Dependency<beGraphics::Texture*>* TextureCache::GetDependencies(const beGraphics::TextureView &texture)
{
	return DX11::GetDependencies( *m, ToImpl(texture).GetResource() );
}

// Method called whenever an observed texture has changed.
void TextureCache::M::ObservedTexture::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	// MONITOR: Non-atomic asynchronous assignment!
	this->pTexture = ToImpl( CreateTexture( LoadTexture(*this->pCache->m, *this->file, this->bSRGB).get(), this->pCache ).get() );
	this->pTextureView = nullptr;
	
	// Notify dependent listeners
	this->pCache->m->dependencies.DependencyChanged(this->dependency, this->pTexture);
}

/// Gets the path resolver.
const beCore::PathResolver& TextureCache::GetPathResolver() const
{
	return m->resolver;
}

} // namespace

// Creates a new texture cache.
lean::resource_ptr<TextureCache, true> CreateTextureCache(const Device &device, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return lean::bind_resource<TextureCache>(
			new DX11::TextureCache(
				ToImpl(device),
				resolver,
				contentProvider
			)
		);
}

} // namespace