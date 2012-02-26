/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/DX11/beSetup.h"
#include "beGraphics/DX11/beDeviceContext.h"
#include "beGraphics/DX/beError.h"

#include <beCore/bePropertyVisitor.h>
#include <beCore/beGenericSerialization.h>

#include <lean/properties/property_types.h>

#include <lean/logging/log.h>

namespace beGraphics
{

namespace DX11
{

namespace
{

// Don't do this lazily, serialization might require type info before construction of first setup
const uint4 BoolTypeID = beCore::RegisterType<BOOL>();
const uint4 IntTypeID = beCore::RegisterType<INT>();
const uint4 UintTypeID = beCore::RegisterType<UINT>();
const uint4 UlongTypeID = beCore::RegisterType<UINT8>();
const uint4 FloatTypeID = beCore::RegisterType<FLOAT>();
const uint4 DoubleTypeID = beCore::RegisterType<DOUBLE>();

/// Gets a property description from the given effect type description. Returns count of zero if unknown.
PropertyDesc GetPropertyDesc(const D3DX11_EFFECT_TYPE_DESC &typeDesc)
{
	size_t componentCount = max<size_t>(typeDesc.Rows, 1)
		* max<size_t>(typeDesc.Columns, 1)
		* max<size_t>(typeDesc.Elements, 1);

	switch (typeDesc.Type)
	{
	case D3D_SVT_BOOL:
		return PropertyDesc(lean::get_property_type_info<BOOL>(), componentCount, BoolTypeID);

	case D3D_SVT_INT:
		return PropertyDesc(lean::get_property_type_info<INT>(), componentCount, IntTypeID);
	case D3D_SVT_UINT:
		return PropertyDesc(lean::get_property_type_info<UINT>(), componentCount, UintTypeID);
	case D3D_SVT_UINT8:
		return PropertyDesc(lean::get_property_type_info<UINT8>(), componentCount, UlongTypeID);

	case D3D_SVT_FLOAT:
		return PropertyDesc(lean::get_property_type_info<FLOAT>(), componentCount, FloatTypeID);
	case D3D_SVT_DOUBLE:
		return PropertyDesc(lean::get_property_type_info<DOUBLE>(), componentCount, DoubleTypeID);
	}

	return PropertyDesc();
}

/// Gets the setup constant buffer.
ID3DX11EffectConstantBuffer* MaybeGetSetupConstants(ID3DX11Effect *pEffect)
{
	ID3DX11EffectConstantBuffer *pConstants = pEffect->GetConstantBufferByName("SetupConstants");
	return (pConstants->IsValid()) ? pConstants : nullptr;
}

/// Clones the given constant buffer, if valid.
lean::com_ptr<ID3D11Buffer, true> MaybeCloneConstantBuffer(ID3DX11EffectConstantBuffer *pConstants)
{
	lean::com_ptr<ID3D11Buffer> pClone;

	if (pConstants)
	{
		lean::com_ptr<ID3D11Buffer> pOriginal;
		BE_THROW_DX_ERROR_MSG(
			pConstants->GetConstantBuffer(pOriginal.rebind()),
			"ID3DX11EffectConstantBuffer::GetConstantBuffer()" );

		if (pOriginal)
		{
			D3D11_BUFFER_DESC desc;
			pOriginal->GetDesc(&desc);

			lean::com_ptr<ID3D11Device> pDevice;
			pOriginal->GetDevice(pDevice.rebind());
		
			BE_THROW_DX_ERROR_MSG(
				pDevice->CreateBuffer(&desc, nullptr, pClone.rebind()),
				"ID3D11Device::CreateBuffer()" );
		}
	}

	return pClone.transfer();
}

// Gets all properties.
Setup::property_vector GetProperties(ID3DX11Effect *pEffect, ID3DX11EffectConstantBuffer *pConstants, ID3D11Buffer *pConstantBuffer,
	lean::scoped_ptr<char[]> &pBackingStore)
{
	Setup::property_vector properties;

	D3DX11_EFFECT_DESC effectDesc;
	BE_THROW_DX_ERROR_MSG(pEffect->GetDesc(&effectDesc), "ID3DX11Effect::GetDesc()");

	Setup::property_vector tempProperties;
	tempProperties.reserve(effectDesc.GlobalVariables);

	bool bHasBuffered = (pConstants && pConstantBuffer);
	size_t constantBufferOffset = 0;
	size_t constantDataSize = 0;

	if (bHasBuffered)
	{
		D3D11_BUFFER_DESC desc;
		pConstantBuffer->GetDesc(&desc);
		constantBufferOffset = desc.ByteWidth;
		constantDataSize += desc.ByteWidth;
	}

	for (uint4 id = 0; id < effectDesc.GlobalVariables; ++id)
	{
		ID3DX11EffectVariable *pVariable = pEffect->GetVariableByIndex(id);
		
		// Valid variable
		if (!pVariable->IsValid())
			LEAN_THROW_ERROR_MSG("ID3DX11Effect::GetVariableByIndex()");

		D3DX11_EFFECT_VARIABLE_DESC variableDesc;
		BE_THROW_DX_ERROR_MSG(pVariable->GetDesc(&variableDesc), "ID3DX11EffectVariable::GetDesc()");
		
		// Not bound by semantic
		if (variableDesc.Semantic)
			continue;

		ID3DX11EffectType *pType = pVariable->GetType();

		// Of valid type
		if (!pType->IsValid())
			LEAN_THROW_ERROR_MSG("ID3DX11EffectVariable::GetType()");

		D3DX11_EFFECT_TYPE_DESC typeDesc;
		BE_THROW_DX_ERROR_MSG(pType->GetDesc(&typeDesc), "ID3DX11EffectType::GetDesc()");

		// Some kind of numeric primitive
		if (typeDesc.Class != D3D_SVC_SCALAR && typeDesc.Class != D3D_SVC_VECTOR &&
			typeDesc.Class != D3D_SVC_MATRIX_ROWS && typeDesc.Class != D3D_SVC_MATRIX_COLUMNS)
			continue;

		PropertyDesc propertyDesc = GetPropertyDesc(typeDesc);

		// Of valid numeric base type
		if (!propertyDesc.Count)
			continue;

		const char *propertyName = variableDesc.Name;
		SUCCEEDED(pVariable->GetAnnotationByName("UIName")->AsString()->GetString(&propertyName))
			|| SUCCEEDED(pVariable->GetAnnotationByName("Name")->AsString()->GetString(&propertyName));

		uint4 bufferOffset = (bHasBuffered && pVariable->GetParentConstantBuffer() == pConstants)
			? variableDesc.BufferOffset
			: Setup::PropertyData::InvalidBufferOffset;

		tempProperties.push_back(
				Setup::Property(
						propertyName,
						propertyDesc,
						Setup::PropertyData(pVariable, nullptr, typeDesc.UnpackedSize, static_cast<uint4>(typeDesc.PackedSize / propertyDesc.Count), bufferOffset)
					)
			);
		
		// Need additional memory, if unbuffered
		if (bufferOffset == Setup::PropertyData::InvalidBufferOffset)
			constantDataSize += typeDesc.UnpackedSize;
	}

	// Allocate fitting properties vector
	properties.reserve(tempProperties.size());

	// Allocate backing store memory
	pBackingStore = new char[constantDataSize];

	// Locate unbuffered properties right after buffered properties
	char *pBackingStoreOffset = pBackingStore + constantBufferOffset;

	// Prepend unbuffered properties
	for (Setup::property_vector::iterator itProperty = tempProperties.begin();
		itProperty != tempProperties.end(); ++itProperty)
	{
		Setup::Property &tempProperty = *itProperty;

		if (!tempProperty.data.IsBuffered())
		{
			properties.push_back(
				Setup::Property(
						tempProperty.name,
						tempProperty.desc,
						Setup::PropertyData(tempProperty.data.pVariable, pBackingStoreOffset,
							tempProperty.data.size, tempProperty.data.elementSize, tempProperty.data.bufferOffset)
					)
				);

			tempProperty.data.pVariable->GetRawValue(pBackingStoreOffset, 0, tempProperty.data.size);

			pBackingStoreOffset += tempProperty.data.size;
		}
	}

	if (bHasBuffered)
	{
		// Append buffered properties
		for (Setup::property_vector::iterator itProperty = tempProperties.begin();
			itProperty != tempProperties.end(); ++itProperty)
		{
			Setup::Property &tempProperty = *itProperty;

			if (tempProperty.data.IsBuffered())
			{
				properties.push_back(
					Setup::Property(
							tempProperty.name,
							tempProperty.desc,
							Setup::PropertyData(tempProperty.data.pVariable, pBackingStore + tempProperty.data.bufferOffset,
								tempProperty.data.size, tempProperty.data.elementSize, tempProperty.data.bufferOffset)
						)
					);

				tempProperty.data.pVariable->GetRawValue(pBackingStore + tempProperty.data.bufferOffset, 0, tempProperty.data.size);
			}
		}
	}

	return properties;
}

// Gets all textures.
Setup::texture_vector GetTextures(ID3DX11Effect *pEffect)
{
	Setup::texture_vector textures;

	D3DX11_EFFECT_DESC effectDesc;
	BE_THROW_DX_ERROR_MSG(pEffect->GetDesc(&effectDesc), "ID3DX11Effect::GetDesc()");

	for (uint4 id = 0; id < effectDesc.GlobalVariables; ++id)
	{
		ID3DX11EffectShaderResourceVariable *pVariable = pEffect->GetVariableByIndex(id)->AsShaderResource();
		
		// Valid texture
		if (!pVariable->IsValid())
			continue;

		D3DX11_EFFECT_VARIABLE_DESC variableDesc;
		BE_THROW_DX_ERROR_MSG(pVariable->GetDesc(&variableDesc), "ID3DX11EffectVariable::GetDesc()");
		
		// Not bound by semantic
		if (variableDesc.Semantic)
			continue;

		const char *textureName = variableDesc.Name;
		SUCCEEDED(pVariable->GetAnnotationByName("UIName")->AsString()->GetString(&textureName))
			|| SUCCEEDED(pVariable->GetAnnotationByName("Name")->AsString()->GetString(&textureName));

		textures.push_back(
			Setup::Texture(
					textureName,
					pVariable,
					nullptr
				)
			);
	}

	return textures;
}

} // namespace

// Constructor.
Setup::Setup(const Effect *pEffect)
	: m_pEffect( LEAN_ASSERT_NOT_NULL(pEffect) ),
	m_pConstants( MaybeGetSetupConstants(*m_pEffect) ),
	m_pConstantBuffer( MaybeCloneConstantBuffer(m_pConstants) ),
	m_properties( GetProperties(*m_pEffect, m_pConstants, m_pConstantBuffer, m_pBackingStore) ),
	m_textures( GetTextures(*m_pEffect) ),
	m_bPropertiesChanged( true )
{
}

// Constructor.
Setup::Setup(ID3DX11Effect *pEffect)
	: m_pEffect( lean::bind_resource(new Effect(pEffect)) ),
	m_pConstants( MaybeGetSetupConstants(*m_pEffect) ),
	m_pConstantBuffer( MaybeCloneConstantBuffer(m_pConstants) ),
	m_properties( GetProperties(*m_pEffect, m_pConstants, m_pConstantBuffer, m_pBackingStore) ),
	m_textures( GetTextures(*m_pEffect) ),
	m_bPropertiesChanged( true )
{
}

// Destructor.
Setup::~Setup()
{
}

// Applys the setup.
void Setup::Apply(const beGraphics::DeviceContext &context) const
{
	// Update buffered properties on demand
	if (m_pConstants)
	{
		if (m_bPropertiesChanged)
		{
			ToImpl(context)->UpdateSubresource(m_pConstantBuffer, 0, nullptr, m_pBackingStore, 0, 0);
			m_bPropertiesChanged = false;
		}
		m_pConstants->SetConstantBuffer(m_pConstantBuffer);
	}

	// Set unbuffered properties
	for (property_vector::const_iterator it = m_properties.begin(); it != m_properties.end() && !it->data.IsBuffered(); ++it)
	{
		const PropertyData &propertyData = it->data;

		propertyData.pVariable->SetRawValue(propertyData.pData, 0, propertyData.size);
	}

	// Set textures
	for (texture_vector::const_iterator it = m_textures.begin(); it != m_textures.end(); ++it)
	{
		const Texture &texture = *it;

		texture.pTexture->SetResource( (texture.pView) ? texture.pView->GetView() : nullptr );
	}
}

// Gets the number of properties.
uint4 Setup::GetPropertyCount() const
{
	return static_cast<uint4>(m_properties.size());
}

// Gets the ID of the given property.
uint4 Setup::GetPropertyID(const utf8_ntri &name) const
{
	for (property_vector::const_iterator it = m_properties.begin();
		it != m_properties.end(); ++it)
		if (it->name == name)
			return static_cast<uint4>(it - m_properties.begin());

	return static_cast<uint4>(-1);
}

// Gets the name of the given property.
utf8_ntr Setup::GetPropertyName(uint4 id) const
{
	return (id < m_properties.size())
		? utf8_ntr(m_properties[id].name)
		: utf8_ntr("");
}

// Gets the type of the given property.
PropertyDesc Setup::GetPropertyDesc(uint4 id) const
{
	return (id < m_properties.size())
		? m_properties[id].desc
		: PropertyDesc();
}

// Sets the given (raw) values.
bool Setup::SetProperty(uint4 id, const std::type_info &type, const void *values, size_t count)
{
	if (id < m_properties.size())
	{
		Property &property = m_properties[id];

		// TODO: Realtime Debugging?
		if (property.desc.TypeInfo->type == type)
		{
			memcpy(property.data.pData, values, min(count, property.desc.Count) * property.data.elementSize);
			m_bPropertiesChanged |= property.data.IsBuffered();
			EmitPropertyChanged();
			return true;
		}
	}
	return false;
}

// Gets the given number of (raw) values.
bool Setup::GetProperty(uint4 id, const std::type_info &type, void *values, size_t count) const
{
	if (id < m_properties.size())
	{
		const Property &property = m_properties[id];

		if (property.desc.TypeInfo->type == type)
		{
			memcpy(values, property.data.pData, min(count, property.desc.Count) * property.data.elementSize);
			return true;
		}
	}
	return false;
}

// Visits a property for modification.
bool Setup::WriteProperty(uint4 id, beCore::PropertyVisitor &visitor, bool bWriteOnly)
{
	if (id < m_properties.size())
	{
		const Property &property = m_properties[id];

		bool bModified = visitor.Visit(*this,id,
			property.desc,
			property.data.pData);

		if (bModified)
		{
			m_bPropertiesChanged |= property.data.IsBuffered();
			EmitPropertyChanged();
		}

		return true;
	}

	return false;
}

// Visits a property for reading.
bool Setup::ReadProperty(uint4 id, beCore::PropertyVisitor &visitor) const
{
	if (id < m_properties.size())
	{
		const Property &property = m_properties[id];

		// WARNING: Call read-only overload!
		visitor.Visit(*this, id,
			property.desc,
			const_cast<const void*>(property.data.pData));

		return true;
	}

	return false;
}

// Gets the type index.
const beCore::TypeIndex* Setup::GetPropertyTypeIndex() const
{
	return &beCore::GetTypeIndex();
}

// Gets the number of textures.
uint4 Setup::GetTextureCount() const
{
	return static_cast<uint4>( m_textures.size() );
}

// Gets the ID of the given texture.
uint4 Setup::GetTextureID(const utf8_ntri &name) const
{
	for (texture_vector::const_iterator it = m_textures.begin();
		it != m_textures.end(); ++it)
		if (it->name == name)
			return static_cast<uint4>( it - m_textures.begin() );

	return static_cast<uint4>(-1);
}

// Gets the name of the given texture.
utf8_ntr Setup::GetTextureName(uint4 id) const
{
	return (id < m_textures.size())
		? utf8_ntr(m_textures[id].name)
		: utf8_ntr("");
}

// Sets the given texture.
void Setup::SetTexture(uint4 id, const beGraphics::TextureView *pView)
{
	if (id < m_textures.size())
	{
		m_textures[id].pView = ToImpl(pView);
		EmitPropertyChanged();
	}
}

// Gets the given texture.
const TextureView* Setup::GetTexture(uint4 id) const
{
	return (id < m_textures.size())
		? m_textures[id].pView
		: nullptr;
}

} // namespace

} // namespace
