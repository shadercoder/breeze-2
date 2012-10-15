/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/DX11/beSetup.h"
#include "beGraphics/DX11/beDeviceContext.h"
#include "beGraphics/DX11/beDevice.h"

#include "beGraphics/beTextureCache.h"
#include "beGraphics/DX11/beBuffer.h"

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
PropertyDesc GetPropertyDesc(const D3DX11_EFFECT_TYPE_DESC &typeDesc, int2 widget)
{
	size_t componentCount = max<size_t>(typeDesc.Rows, 1)
		* max<size_t>(typeDesc.Columns, 1)
		* max<size_t>(typeDesc.Elements, 1);

	switch (typeDesc.Type)
	{
	case D3D_SVT_BOOL:
		return PropertyDesc(lean::get_property_type_info<BOOL>(), componentCount, BoolTypeID, widget);

	case D3D_SVT_INT:
		return PropertyDesc(lean::get_property_type_info<INT>(), componentCount, IntTypeID, widget);
	case D3D_SVT_UINT:
		return PropertyDesc(lean::get_property_type_info<UINT>(), componentCount, UintTypeID, widget);
	case D3D_SVT_UINT8:
		return PropertyDesc(lean::get_property_type_info<UINT8>(), componentCount, UlongTypeID, widget);

	case D3D_SVT_FLOAT:
		return PropertyDesc(lean::get_property_type_info<FLOAT>(), componentCount, FloatTypeID, widget);
	case D3D_SVT_DOUBLE:
		return PropertyDesc(lean::get_property_type_info<DOUBLE>(), componentCount, DoubleTypeID, widget);
	}

	return PropertyDesc();
}

/// Gets the widget by name.
int2 GetWidgetByName(const utf8_ntri &name)
{
	if (_stricmp(name.c_str(), "color") == 0)
		return beCore::Widget::Color;
	else if (_stricmp(name.c_str(), "slider") == 0)
		return beCore::Widget::Slider;
	else if (_stricmp(name.c_str(), "raw") == 0)
		return beCore::Widget::Raw;
	else if (_stricmp(name.c_str(), "angle") == 0)
		return beCore::Widget::Angle;
	else if (_stricmp(name.c_str(), "none") == 0)
		return beCore::Widget::None;
	else if (_stricmp(name.c_str(), "orientation") == 0)
		return beCore::Widget::Orientation;
	else
		// Default to raw value
		return beCore::Widget::Raw;
}

/// Gets the setup constant buffer.
ID3DX11EffectConstantBuffer* MaybeGetSetupConstants(ID3DX11Effect *pEffect)
{
	ID3DX11EffectConstantBuffer *pConstants = pEffect->GetConstantBufferByName("SetupConstants");
	return (pConstants->IsValid()) ? pConstants : nullptr;
}

/// Clones the given constant buffer, if valid.
lean::com_ptr<ID3D11Buffer, true> MaybeCloneConstantBuffer(ID3DX11EffectConstantBuffer *pConstants, uint4 &bufferSize)
{
	lean::com_ptr<ID3D11Buffer> pClone;
	bufferSize = 0;

	if (pConstants)
	{
		lean::com_ptr<ID3D11Buffer> pOriginal;
		BE_THROW_DX_ERROR_MSG(
			pConstants->GetConstantBuffer(pOriginal.rebind()),
			"ID3DX11EffectConstantBuffer::GetConstantBuffer()" );

		if (pOriginal)
		{
			D3D11_BUFFER_DESC desc = GetDesc(pOriginal);

			pClone = CreateBuffer(desc, nullptr, GetDevice(*pOriginal));
			bufferSize = desc.ByteWidth;
		}
	}

	return pClone.transfer();
}

// Gets all properties.
Setup::property_vector GetProperties(ID3DX11Effect *pEffect,
	ID3DX11EffectConstantBuffer *pConstants, ID3D11Buffer *pConstantBuffer, uint4 constantBufferSize,
	uint4 &unbufferedBeginID, uint4 &unbufferedEndID,
	lean::scoped_ptr<char[]> &backingStore, uint4 &backingStoreSize)
{
	Setup::property_vector properties;

	D3DX11_EFFECT_DESC effectDesc = GetDesc(pEffect);
	properties.reserve(effectDesc.GlobalVariables);

	// Allocate dedicated block of memory for buffered constants only
	uint4 globalConstantOffset = constantBufferSize;

	bool bHasBuffered = (pConstants && pConstantBuffer);

	unbufferedBeginID = static_cast<uint4>(-1);
	unbufferedEndID = 0;

	for (uint4 id = 0; id < effectDesc.GlobalVariables; ++id)
	{
		ID3DX11EffectVariable *pVariable = pEffect->GetVariableByIndex(id);
		// Valid variable
		D3DX11_EFFECT_VARIABLE_DESC variableDesc = GetDesc(pVariable);
		
		// Not bound by semantic
		if (variableDesc.Semantic)
			continue;

		ID3DX11EffectType *pType = pVariable->GetType();
		// Of valid type
		D3DX11_EFFECT_TYPE_DESC typeDesc = GetDesc(pType);

		// Some kind of numeric primitive
		if (typeDesc.Class != D3D_SVC_SCALAR && typeDesc.Class != D3D_SVC_VECTOR &&
			typeDesc.Class != D3D_SVC_MATRIX_ROWS && typeDesc.Class != D3D_SVC_MATRIX_COLUMNS)
			continue;

		const char *widgetName = nullptr;
		SUCCEEDED(pVariable->GetAnnotationByName("UIWidget")->AsString()->GetString(&widgetName))
			|| SUCCEEDED(pVariable->GetAnnotationByName("Widget")->AsString()->GetString(&widgetName));

		int2 widget = (widgetName) ? GetWidgetByName(widgetName) : beCore::Widget::Raw;
		PropertyDesc propertyDesc = GetPropertyDesc(typeDesc, widget);

		// Of valid numeric base type
		if (!propertyDesc.Count)
			continue;

		const char *propertyName = variableDesc.Name;
		SUCCEEDED(pVariable->GetAnnotationByName("UIName")->AsString()->GetString(&propertyName))
			|| SUCCEEDED(pVariable->GetAnnotationByName("Name")->AsString()->GetString(&propertyName));

		// Add to buffered or to unbuffered block
		bool bBuffered = (bHasBuffered && pVariable->GetParentConstantBuffer() == pConstants);
		uint4 backingStoreOffset = (bBuffered) ? variableDesc.BufferOffset : globalConstantOffset;

		uint4 propertyID = static_cast<uint4>(properties.size());
		
		properties.push_back(
				Setup::Property(
						propertyName,
						propertyDesc,
						Setup::PropertyData(
								pVariable,
								backingStoreOffset,
								static_cast<uint2>(typeDesc.UnpackedSize),
								static_cast<uint2>(typeDesc.PackedSize / propertyDesc.Count)
							)
					)
			);
		
		if (!bBuffered)
		{
			// Need additional memory, if unbuffered
			globalConstantOffset += typeDesc.UnpackedSize;

			unbufferedBeginID = min(propertyID, unbufferedBeginID);
			unbufferedEndID = max(propertyID + 1, unbufferedEndID);
		}
	}

	// Allocate fitting properties vector
	properties.shrink_to_fit();

	// Allocate backing store memory
	backingStoreSize = globalConstantOffset;
	backingStore = new char[backingStoreSize];

	// Get default values
	for (Setup::property_vector::const_iterator it = properties.begin(); it != properties.end(); ++it)
		it->data.pVariable->GetRawValue(backingStore + it->data.offset, 0, it->data.size);

	return properties;
}

// Gets all textures.
Setup::texture_vector GetTextures(ID3DX11Effect *pEffect, beGraphics::TextureCache *pTextures)
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
		
		// Not bound by semantic or unmanaged
		if (variableDesc.Semantic || (variableDesc.Flags & D3DX11_EFFECT_VARIABLE_UNMANAGED))
			continue;

		ID3DX11EffectType *pType = pVariable->GetType();
		// Of valid type
		D3DX11_EFFECT_TYPE_DESC typeDesc = GetDesc(pType);

		// Some kind of texture
		if (typeDesc.Type != D3D_SVT_TEXTURE &&
			typeDesc.Type != D3D_SVT_TEXTURE1D && typeDesc.Type != D3D_SVT_TEXTURE2D &&
			typeDesc.Type != D3D_SVT_TEXTURE3D  && typeDesc.Type != D3D_SVT_TEXTURECUBE)
			continue;

		BOOL isRaw = FALSE;
		pVariable->GetAnnotationByName("Raw")->AsScalar()->GetBool(&isRaw);

		const char *textureName = variableDesc.Name;
		SUCCEEDED(pVariable->GetAnnotationByName("UIName")->AsString()->GetString(&textureName))
			|| SUCCEEDED(pVariable->GetAnnotationByName("Name")->AsString()->GetString(&textureName));

		const char *textureFile = nullptr;
		SUCCEEDED(pVariable->GetAnnotationByName("UIFile")->AsString()->GetString(&textureFile))
			|| SUCCEEDED(pVariable->GetAnnotationByName("File")->AsString()->GetString(&textureFile));

		const beGraphics::TextureView *pView = (pTextures && textureFile)
			? pTextures->GetTextureView(textureFile, !isRaw)
			: nullptr;

		textures.push_back(
			Setup::Texture(
					textureName,
					pVariable,
					ToImpl(pView),
					!isRaw
				)
			);
	}

	return textures;
}

// Gets all properties.
char* CloneBackingStore(const char *storeData, uint4 storeSize)
{
	char* backingStore = new char[storeSize];
	memcpy(backingStore, storeData, storeSize);
	return backingStore;
}

} // namespace

// Constructor.
Setup::Setup(const Effect *pEffect, beGraphics::TextureCache *pTextures)
	: m_pEffect( LEAN_ASSERT_NOT_NULL(pEffect) ),
	m_pConstants( MaybeGetSetupConstants(*m_pEffect) ),
	m_pConstantBuffer( MaybeCloneConstantBuffer(m_pConstants, m_constantBufferSize) ),
	m_properties(
		GetProperties(
				*m_pEffect,
				m_pConstants, m_pConstantBuffer, m_constantBufferSize,
				m_unbufferedPropertiesBegin, m_unbufferedPropertiesEnd,
				m_backingStore, m_backingStoreSize
			)
		),
	m_textures( GetTextures(*m_pEffect, pTextures) ),
	m_bPropertiesChanged( true )
{
}

// Copy constructor.
Setup::Setup(const Setup &right)
	: m_pEffect( right.m_pEffect ),
	m_backingStoreSize( right.m_backingStoreSize ),
	m_backingStore( CloneBackingStore(right.m_backingStore, right.m_backingStoreSize) ),
	m_pConstants( right.m_pConstants ),
	m_pConstantBuffer( MaybeCloneConstantBuffer(m_pConstants, m_constantBufferSize) ),
	m_unbufferedPropertiesBegin( right.m_unbufferedPropertiesBegin ),
	m_unbufferedPropertiesEnd( right.m_unbufferedPropertiesEnd ),
	m_properties( right.m_properties ),
	m_textures( right.m_textures ),
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
			ToImpl(context)->UpdateSubresource(m_pConstantBuffer, 0, nullptr, m_backingStore, 0, 0);
			m_bPropertiesChanged = false;
		}
		m_pConstants->SetConstantBuffer(m_pConstantBuffer);
	}

	// Set unbuffered properties
	for (uint4 i = m_unbufferedPropertiesBegin; i < m_unbufferedPropertiesEnd; ++i)
	{
		const PropertyData &propertyData = m_properties[i].data;

		// Check if unbuffered
		if (propertyData.offset >= m_constantBufferSize)
			propertyData.pVariable->SetRawValue(m_backingStore + propertyData.offset, 0, propertyData.size);
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
			memcpy(m_backingStore + property.data.offset, values, min(count, property.desc.Count) * property.data.elementSize);
			// Check if buffered
			m_bPropertiesChanged |= (property.data.offset < m_constantBufferSize);
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
			memcpy(values, m_backingStore + property.data.offset, min(count, property.desc.Count) * property.data.elementSize);
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

		bool bModified = visitor.Visit(
				*this,
				id,
				property.desc,
				m_backingStore + property.data.offset
			);

		if (bModified)
		{
			// Check if buffered
			m_bPropertiesChanged |= (property.data.offset < m_constantBufferSize);
			EmitPropertyChanged();
		}

		return true;
	}

	return false;
}

// Visits a property for reading.
bool Setup::ReadProperty(uint4 id, beCore::PropertyVisitor &visitor, bool bPersistentOnly) const
{
	if (id < m_properties.size())
	{
		const Property &property = m_properties[id];

		// WARNING: Call read-only overload!
		visitor.Visit(
				*this,
				id,
				property.desc,
				const_cast<const char*>(m_backingStore + property.data.offset)
			);

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

// Gets whether the texture is a color texture.
bool Setup::IsColorTexture(uint4 id) const
{
	return (id < m_textures.size())
		? m_textures[id].bColorTexture
		: true;
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

// Updates the given texture.
void Setup::DependencyChanged(beGraphics::Texture *oldTexture, beGraphics::Texture *newTexture)
{
	ID3D11Resource *oldTextureResource = ToImpl(oldTexture)->GetResource();

	for (texture_vector::const_iterator it = m_textures.begin(); it != m_textures.end(); ++it)
		// Find & replace textures
		if (it->pView->GetResource() == oldTextureResource)
			SetTexture(
					static_cast<uint4>(it - m_textures.begin()),
					LEAN_ASSERT_NOT_NULL(newTexture->GetCache())->GetTextureView(*newTexture)
				);
}

// Gets the number of child components.
uint4 Setup::GetComponentCount() const
{
	return GetTextureCount();
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string Setup::GetComponentName(uint4 idx) const
{
	beCore::Exchange::utf8_string name;

	utf8_ntr textureName = GetTextureName(idx);
	name.reserve(textureName.size() + lean::ntarraylen(" (Texture)"));

	name.append(textureName.c_str(), textureName.size());
	name.append(" (Texture)", lean::ntarraylen(" (Texture)"));

	return name;
}

// Gets the n-th reflected child component, nullptr if not reflected.
const beCore::ReflectedComponent* Setup::GetReflectedComponent(uint4 idx) const
{
	return nullptr;
}

// Gets the type of the n-th child component.
beCore::Exchange::utf8_string Setup::GetComponentType(uint4 idx) const
{
	return "Texture";
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> Setup::GetComponent(uint4 idx) const
{
	return lean::any_value<beGraphics::TextureView*>( const_cast<beGraphics::DX11::TextureView*>( GetTexture(idx) ) );
}

// Returns true, if the n-th component can be replaced.
bool Setup::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void Setup::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetTexture(idx, lean::any_cast<beGraphics::TextureView*>(pComponent));
}

} // namespace

} // namespace
