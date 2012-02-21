/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#include "beGraphicsInternal/stdafx.h"

#define BE_GRAPHICS_TEXTURE_DX11_INSTANTIATE

#include "beGraphics/DX11/beTexture.h"
#include "beGraphics/DX11/beFormat.h"
#include "beGraphics/DX11/beDevice.h"
#include "beGraphics/DX/beError.h"
#include <D3DX11.h>
#include <lean/strings/conversions.h>

namespace beGraphics
{

namespace DX11
{

/// Constructs a D3DX11 load info from the given texture description.
D3DX11_IMAGE_LOAD_INFO ToD3DX11(const TextureDesc& desc)
{
	D3DX11_IMAGE_LOAD_INFO loadInfo;
	loadInfo.Width = (desc.Width != 0) ? desc.Width : D3DX11_DEFAULT;
	loadInfo.Height = (desc.Height != 0) ? desc.Height : D3DX11_DEFAULT;
	loadInfo.Depth = (desc.Depth != 0) ? desc.Depth : D3DX11_DEFAULT;
	loadInfo.FirstMipLevel = D3DX11_DEFAULT;
	loadInfo.MipLevels = (desc.MipLevels != 0) ? desc.MipLevels : D3DX11_DEFAULT;
	loadInfo.Usage = static_cast<D3D11_USAGE>(D3DX11_DEFAULT);
	loadInfo.BindFlags  = D3DX11_DEFAULT;
	loadInfo.CpuAccessFlags  = D3DX11_DEFAULT;
	loadInfo.MiscFlags  = D3DX11_DEFAULT;
	loadInfo.Format  = (desc.Format != Format::Unknown) ? ToAPI(desc.Format) : DXGI_FORMAT_FROM_FILE;
	loadInfo.Filter = D3DX11_DEFAULT;
	loadInfo.MipFilter = D3DX11_DEFAULT;
	loadInfo.pSrcInfo = nullptr;
	return loadInfo;
}

// Loads a texture from the given file.
lean::com_ptr<ID3D11Resource, true> LoadTexture(const lean::utf8_ntri &fileName, const TextureDesc *pDesc, ID3D11Device *pDevice)
{
	lean::com_ptr<ID3D11Resource> pTexture;

	BE_THROW_DX_ERROR_CTX(
		::D3DX11CreateTextureFromFile(
			pDevice,
			lean::utf_to_utf16(fileName).c_str(),
			(pDesc) ? &ToD3DX11(*pDesc) : nullptr,
			nullptr,
			pTexture.rebind(),
			nullptr),
		"D3DX11CreateTextureFromFile()",
		fileName.c_str() );

	return pTexture.transfer();
}

// Loads a texture from the given memory.
lean::com_ptr<ID3D11Resource, true> LoadTexture(const char *data, uint4 dataLength, const TextureDesc *pDesc, ID3D11Device *pDevice)
{
	lean::com_ptr<ID3D11Resource> pTexture;

	BE_THROW_DX_ERROR_MSG(
		::D3DX11CreateTextureFromMemory(
			pDevice,
			data,
			dataLength,
			(pDesc) ? &ToD3DX11(*pDesc) : nullptr,
			nullptr,
			pTexture.rebind(),
			nullptr),
		"D3DX11CreateTextureFromMemory()" );

	return pTexture.transfer();
}

// Creates a texture from the given texture resource.
lean::resource_ptr<beGraphics::Texture, true> CreateTexture(ID3D11Resource *pTextureResource, beGraphics::TextureCache *pCache)
{
	LEAN_ASSERT_NOT_NULL(pTextureResource);

	Texture *pTexture = nullptr;

	D3D11_RESOURCE_DIMENSION texDim;
	pTextureResource->GetType(&texDim);

	switch (texDim)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		pTexture = new Texture1D( static_cast<ID3D11Texture1D*>(pTextureResource), pCache );
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		pTexture = new Texture2D( static_cast<ID3D11Texture2D*>(pTextureResource), pCache );
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		pTexture = new Texture3D( static_cast<ID3D11Texture3D*>(pTextureResource), pCache );
		break;
	default:
		LEAN_THROW_ERROR_MSG("Invalid texture resource type!");
	}

	return lean::bind_resource<beGraphics::Texture>(pTexture);
}

} // namespace

// Loads a texture from the given file.
lean::resource_ptr<beGraphics::Texture, true> LoadTexture(const lean::utf8_ntri &fileName, const TextureDesc *pDesc, const beGraphics::Device &device, TextureCache *pCache)
{
	return DX11::CreateTexture( DX11::LoadTexture(fileName, pDesc, ToImpl(device).Get()).get(), pCache );
}

// Loads a texture from the given memory.
lean::resource_ptr<beGraphics::Texture, true> LoadTexture(const char *data, uint4 dataLength, const TextureDesc *pDesc, const beGraphics::Device &device, TextureCache *pCache)
{
	return DX11::CreateTexture( DX11::LoadTexture(data, dataLength, pDesc, ToImpl(device).Get()).get(), pCache );
}

// Creates a texture view from the given texture.
lean::resource_ptr<beGraphics::TextureView, true> ViewTexture(const beGraphics::Texture &texture, const beGraphics::Device &device)
{
	return lean::bind_resource<TextureView>(
			new DX11::TextureView( ToImpl(texture).GetResource(), nullptr, ToImpl(device) )
		);
}

// Gets the back buffer.
lean::resource_ptr<Texture, true> GetBackBuffer(const SwapChain &swapChain, uint4 index)
{
	return lean::bind_resource( new DX11::Texture2D( DX11::GetBackBuffer(ToImpl(swapChain).Get(), index).get() ) );
}

namespace DX11
{

// Creates a texture from the given description.
lean::com_ptr<ID3D11Texture1D, true> CreateTexture(const D3D11_TEXTURE1D_DESC &desc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Device *pDevice)
{
	LEAN_ASSERT(pDevice != nullptr);

	lean::com_ptr<ID3D11Texture1D> pTexture;

	BE_THROW_DX_ERROR_MSG(
		pDevice->CreateTexture1D(&desc, pInitialData, pTexture.rebind()),
		"ID3D11Device::CreateTexture1D()");

	return pTexture.transfer();
}

// Creates a texture from the given description.
lean::com_ptr<ID3D11Texture2D, true> CreateTexture(const D3D11_TEXTURE2D_DESC &desc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Device *pDevice)
{
	LEAN_ASSERT(pDevice != nullptr);

	lean::com_ptr<ID3D11Texture2D> pTexture;

	BE_THROW_DX_ERROR_MSG(
		pDevice->CreateTexture2D(&desc, pInitialData, pTexture.rebind()),
		"ID3D11Device::CreateTexture2D()");

	return pTexture.transfer();
}

// Creates a texture from the given description.
lean::com_ptr<ID3D11Texture3D, true> CreateTexture(const D3D11_TEXTURE3D_DESC &desc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Device *pDevice)
{
	LEAN_ASSERT(pDevice != nullptr);

	lean::com_ptr<ID3D11Texture3D> pTexture;

	BE_THROW_DX_ERROR_MSG(
		pDevice->CreateTexture3D(&desc, pInitialData, pTexture.rebind()),
		"ID3D11Device::CreateTexture3D()");

	return pTexture.transfer();
}

// Creates a shader resource view from the given texture.
lean::com_ptr<ID3D11ShaderResourceView, true> CreateShaderResourceView(ID3D11Resource *pTexture, const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc, ID3D11Device *pDevice)
{
	LEAN_ASSERT(pTexture != nullptr);
	LEAN_ASSERT(pDevice != nullptr);

	lean::com_ptr<ID3D11ShaderResourceView> pView;

	BE_THROW_DX_ERROR_MSG(
		pDevice->CreateShaderResourceView(pTexture, pDesc, pView.rebind()),
		"ID3D11Device::CreateShaderResourceView()");

	return pView.transfer();
}

// Constructor.
template <TextureType::T Type>
TypedTexture<Type>::TypedTexture(const DescType &desc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Device *pDevice, beGraphics::TextureCache *pCache)
	: Texture(pCache),
	m_pTexture( CreateTexture(desc, pInitialData, pDevice) )
{
}

// Constructor.
template <TextureType::T Type>
TypedTexture<Type>::TypedTexture(InterfaceType *pTexture, beGraphics::TextureCache *pCache)
	: Texture(pCache),
	m_pTexture(pTexture)
{
	LEAN_ASSERT(m_pTexture != nullptr);
}

// Destructor.
template <TextureType::T Type>
TypedTexture<Type>::~TypedTexture()
{
}

// Constructor.
TextureView::TextureView(ID3D11Resource *pTexture, const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc, ID3D11Device *pDevice, beGraphics::TextureCache *pCache)
	: m_pTexture( CreateShaderResourceView(pTexture, pDesc, pDevice) ),
	m_pCache( pCache )
{
}

// Constructor.
TextureView::TextureView(ID3D11ShaderResourceView *pView, beGraphics::TextureCache *pCache)
	: m_pTexture( LEAN_ASSERT_NOT_NULL(pView) ),
	m_pCache( pCache )
{
}

// Destructor.
TextureView::~TextureView()
{
}

// Maps this texture to allow for CPU access.
bool Map(ID3D11DeviceContext *pDeviceContext, ID3D11Resource *pTexture, uint4 subResource,
		D3D11_MAPPED_SUBRESOURCE &data, D3D11_MAP map, uint4 flags)
{
	LEAN_ASSERT(pDeviceContext != nullptr);
	LEAN_ASSERT(pTexture != nullptr);

	bool bSuccess = BE_LOG_DX_ERROR_MSG(
		pDeviceContext->Map(
			pTexture, subResource,
			map, flags,
			&data),
		"ID3D11DeviceContext::Map()");

	if (!bSuccess)
		data.pData = nullptr;

	return bSuccess;
}

// Unmaps this texture to allow for GPU access.
void Unmap(ID3D11DeviceContext *pDeviceContext, ID3D11Resource *pTexture, uint4 subResource)
{
	LEAN_ASSERT(pDeviceContext != nullptr);
	LEAN_ASSERT(pTexture != nullptr);

	pDeviceContext->Unmap(pTexture, subResource);
}

// Gets a description of the given texture.
TextureDesc GetDesc(ID3D11Texture1D *pTexture)
{
	D3D11_TEXTURE1D_DESC desc;
	pTexture->GetDesc(&desc);
	return TextureDesc(desc.Width, 1, 1, ToBE(desc.Format), desc.MipLevels);
}

// Gets a description of the given texture.
TextureDesc GetDesc(ID3D11Texture2D *pTexture)
{
	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);
	return TextureDesc(desc.Width, desc.Height, 1, ToBE(desc.Format), desc.MipLevels);
}

// Gets a description of the given texture.
TextureDesc GetDesc(ID3D11Texture3D *pTexture)
{
	D3D11_TEXTURE3D_DESC desc;
	pTexture->GetDesc(&desc);
	return TextureDesc(desc.Width, desc.Height, desc.Depth, ToBE(desc.Format), desc.MipLevels);
}

// Gets a description of the given texture.
TextureDesc GetDesc(ID3D11Resource *pTexture)
{
	TextureType::T type = GetType(pTexture);

	switch (type)
	{
	case TextureType::Texture1D:
		return GetDesc( static_cast<ID3D11Texture1D*>(pTexture) );
	case TextureType::Texture2D:
		return GetDesc( static_cast<ID3D11Texture2D*>(pTexture) );
	case TextureType::Texture3D:
		return GetDesc( static_cast<ID3D11Texture3D*>(pTexture) );
	}

	return TextureDesc();
}

// Gets the type of the given texture.
TextureType::T GetType(ID3D11Resource *pTexture)
{
	D3D11_RESOURCE_DIMENSION resourceDim;
	pTexture->GetType(&resourceDim);

	switch (resourceDim)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D: return TextureType::Texture1D;
	case D3D11_RESOURCE_DIMENSION_TEXTURE2D: return TextureType::Texture2D;
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D: return TextureType::Texture3D;
	}

	return TextureType::NotATexture;
}

} // namespace

} // namespace
