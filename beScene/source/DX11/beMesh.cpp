/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/DX11/beMesh.h"
#include <beGraphics/Any/beFormat.h>

#include <beMath/beVector.h>
#include <beMath/beSphere.h>

namespace beScene
{
namespace DX11
{

// Constructs a vertex buffer description form the given parameters.
D3D11_BUFFER_DESC GetVertexBufferDesc(uint4 vertexSize, uint4 vertexCount)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = vertexSize * vertexCount;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = vertexSize;
	return desc;
}

// Constructs an index buffer description form the given parameters.
D3D11_BUFFER_DESC GetIndexBufferDesc(uint4 indexSize, uint4 indexCount)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = indexSize * indexCount;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = indexSize;
	return desc;
}

/// Gets the first vertex element of the given semantic. Returns nullptr, if none found.
const D3D11_INPUT_ELEMENT_DESC* GetInputElementBySemantic(const D3D11_INPUT_ELEMENT_DESC *pElementDescs, uint4 elementCount, const char *semanticName)
{
	const D3D11_INPUT_ELEMENT_DESC *pElementDescsEnd = pElementDescs + elementCount;

	for (; pElementDescs != pElementDescsEnd; ++pElementDescs)
		if (lean::char_traits<char>::equal(pElementDescs->SemanticName, semanticName))
			return pElementDescs;

	return nullptr;
}

/// Gets the offset of the given vertex element.
uint4 GetInputElementOffset(const D3D11_INPUT_ELEMENT_DESC *pElementDescs, const D3D11_INPUT_ELEMENT_DESC *pTargetElementDesc)
{
	uint4 offset = 0;

	for (; pElementDescs != pTargetElementDesc; ++pElementDescs)
	{
		if (pElementDescs->AlignedByteOffset != D3D11_APPEND_ALIGNED_ELEMENT)
			offset = pElementDescs->AlignedByteOffset;

		offset += beGraphics::Any::SizeofFormat(pElementDescs->Format);
	}

	return offset;
}

// Computes a bounding sphere from the given vertices.
beMath::fsphere3 ComputeVertexBounds(const D3D11_INPUT_ELEMENT_DESC *pElementDescs, uint4 elementCount,
	uint4 vertexSize, const void *pVertices, uint4 vertexCount)
{
	beMath::fsphere3 bounds;

	beMath::fvec3 boundsMin(FLT_MAX);
	beMath::fvec3 boundsMax(-FLT_MAX);

	const D3D11_INPUT_ELEMENT_DESC *pPositionElementDesc = GetInputElementBySemantic(pElementDescs, elementCount, "POSITION");

	if (pPositionElementDesc)
	{
		uint4 positionOffset = GetInputElementOffset(pElementDescs, pPositionElementDesc);

		const char *pVertexBytes = static_cast<const char*>(pVertices);
		const char *pVertexBytesEnd = pVertexBytes + vertexSize * vertexCount;

		for (; pVertexBytes != pVertexBytesEnd; pVertexBytes += vertexSize)
		{
			const float *pVertexPos = reinterpret_cast<const float*>(pVertexBytes + positionOffset);
			beMath::fvec3 vertexPos = beMath::vec(pVertexPos[0], pVertexPos[1], pVertexPos[2]);

			boundsMin = min_cw(boundsMin, vertexPos);
			boundsMax = max_cw(boundsMax, vertexPos);
		}
	}

	bounds.p() = (boundsMin + boundsMax) * 0.5f;
	bounds.r() = length(boundsMax - boundsMin) * 0.5f;

	return bounds;
}

// Constructor.
Mesh::Mesh(const D3D11_INPUT_ELEMENT_DESC *pElementDescs, uint4 elementCount,
		uint4 vertexSize, const void *pVertices, uint4 vertexCount,
		DXGI_FORMAT indexFormat, const void *pIndices, uint4 indexCount,
		const beMath::fsphere3 &bounds,
		ID3D11Device *pDevice, MeshCompound *pCompound)
	: beScene::Mesh(bounds, pCompound),
	m_vertexElements( pElementDescs, pElementDescs + elementCount ),
	m_vertexBuffer( GetVertexBufferDesc(vertexSize, vertexCount), pVertices, pDevice ),
	m_indexBuffer( GetIndexBufferDesc(beGraphics::Any::SizeofFormat(indexFormat), indexCount), pIndices, pDevice ),
	m_indexFormat(indexFormat),
	m_indexCount(indexCount)
{
}

// Constructor.
Mesh::Mesh(const D3D11_INPUT_ELEMENT_DESC *pElementDescs, uint4 elementCount,
		uint4 vertexSize, const void *pVertices, uint4 vertexCount,
		DXGI_FORMAT indexFormat, const void *pIndices, uint4 indexCount,
		ID3D11Device *pDevice, MeshCompound *pCompound)
	: beScene::Mesh( ComputeVertexBounds(pElementDescs, elementCount, vertexSize, pVertices, vertexCount), pCompound ),
	m_vertexElements( pElementDescs, pElementDescs + elementCount ),
	m_vertexBuffer( GetVertexBufferDesc(vertexSize, vertexCount), pVertices, pDevice ),
	m_vertexSize(vertexSize),
	m_vertexCount(vertexCount),
	m_indexBuffer( GetIndexBufferDesc(beGraphics::Any::SizeofFormat(indexFormat), indexCount), pIndices, pDevice ),
	m_indexFormat(indexFormat),
	m_indexCount(indexCount)
{
}

// Destructor.
Mesh::~Mesh()
{
}

} // namespace

} // namespace