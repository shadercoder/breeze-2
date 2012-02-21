/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/DX11/beBuffer.h"
#include "beGraphics/DX/beError.h"

namespace beGraphics
{

namespace DX11
{

// Creates a buffer according to the given description.
lean::com_ptr<ID3D11Buffer, true> CreateBuffer(const D3D11_BUFFER_DESC &desc, const void *pInitialData, ID3D11Device *pDevice)
{
	lean::com_ptr<ID3D11Buffer> pBuffer;

	D3D11_SUBRESOURCE_DATA initialData = { pInitialData };
	BE_THROW_DX_ERROR_MSG(
		pDevice->CreateBuffer(&desc, pInitialData ? &initialData : nullptr, pBuffer.rebind()),
		"ID3D11Device::CreateBuffer()");

	return pBuffer.transfer();
}

// Constructor.
Buffer::Buffer(const D3D11_BUFFER_DESC &desc, const void *pInitialData, ID3D11Device *pDevice)
	: m_pBuffer( CreateBuffer(desc, pInitialData, pDevice) )
{
}

// Constructor.
Buffer::Buffer(ID3D11Buffer *pBuffer)
	: m_pBuffer(pBuffer)
{
	LEAN_ASSERT(m_pBuffer != nullptr);
}

// Destructor.
Buffer::~Buffer()
{
}

// Updates the given unstructured buffer with the given data.
void PartialUpdate(ID3D11DeviceContext *pDeviceContext, ID3D11Buffer *pBuffer, uint4 offset, uint4 endOffset, const void *pData, uint4 sourceOffset)
{
	LEAN_ASSERT(pDeviceContext != nullptr);
	LEAN_ASSERT(pBuffer != nullptr);
	LEAN_ASSERT(pData != nullptr);

	D3D11_BOX destBox = { offset, 0, 0, endOffset, 1, 1 };

	pDeviceContext->UpdateSubresource(
		pBuffer, 0, &destBox,
		reinterpret_cast<const char*>(&pData) + sourceOffset,
		0, 0);
}

// Maps this buffer to allow for CPU access.
bool Map(ID3D11DeviceContext *pDeviceContext, ID3D11Buffer *pBuffer, void *&data, D3D11_MAP map, uint4 flags)
{
	LEAN_ASSERT(pDeviceContext != nullptr);
	LEAN_ASSERT(pBuffer != nullptr);
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	bool bSuccess = BE_LOG_DX_ERROR_MSG(
		pDeviceContext->Map(
			pBuffer, 0,
			map, flags,
			&mappedResource),
		"ID3D11DeviceContext::Map()");

	data = (bSuccess) ? mappedResource.pData : nullptr;
	return bSuccess;
}

// Unmaps this buffer to allow for GPU access.
void Unmap(ID3D11DeviceContext *pDeviceContext, ID3D11Buffer *pBuffer)
{
	LEAN_ASSERT(pDeviceContext != nullptr);
	LEAN_ASSERT(pBuffer != nullptr);

	pDeviceContext->Unmap(pBuffer, 0);
}

} // namespace

} // namespace