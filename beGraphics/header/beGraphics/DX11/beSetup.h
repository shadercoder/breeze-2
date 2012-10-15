/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_SETUP_DX11
#define BE_GRAPHICS_SETUP_DX11

#include "beGraphics.h"
#include "../beSetup.h"
#include "beEffect.h"
#include "beTexture.h"
#include <D3DX11Effect.h>
#include <lean/smart/resource_ptr.h>
#include <vector>
#include <lean/smart/scoped_ptr.h>

#include <beCore/beDependencies.h>

namespace beGraphics
{

class TextureCache;

namespace DX11
{

using beCore::PropertyDesc;

/// Setup implementation.
class Setup : public beCore::PropertyFeedbackProvider<beGraphics::Setup>, public beCore::Dependent<beGraphics::Texture*>
{
	LEAN_RENEW_RESOURCE

public:
	/// Property data.
	struct PropertyData
	{
		ID3DX11EffectVariable *pVariable;
		uint4 offset;
		uint2 size;
		uint2 elementSize;

		/// Constructor.
		PropertyData(ID3DX11EffectVariable *pVariable, uint4 offset, uint2 size, uint2 elementSize)
			: pVariable(pVariable),
			offset(offset),
			size(size),
			elementSize(elementSize) { }
	};

	/// Property.
	struct Property
	{
		utf8_string name;
		PropertyDesc desc;
		PropertyData data;
		bool bChanged;

		/// Constructor.
		Property(const utf8_ntri& name,
			const PropertyDesc &desc,
			const PropertyData& data)
				: name(name.to<utf8_string>()),
				desc(desc),
				data(data),
				bChanged(false) { }
	};

	typedef std::vector<Property> property_vector;

	/// Texture.
	struct Texture
	{
		utf8_string name;
		ID3DX11EffectShaderResourceVariable *pTexture;
		lean::resource_ptr<const TextureView> pView;
		bool bColorTexture;

		/// Constructor.
		Texture(const utf8_ntri& name,
			ID3DX11EffectShaderResourceVariable *pTexture,
			const TextureView *pView,
			bool bColorTexture)
				: name(name.to<utf8_string>()),
				pTexture(pTexture),
				pView(pView),
				bColorTexture(bColorTexture) { }
	};
	typedef std::vector<Texture> texture_vector;

private:
	lean::resource_ptr<const Effect> m_pEffect;
	
	uint4 m_backingStoreSize;
	lean::scoped_ptr<char[]> m_backingStore;

	ID3DX11EffectConstantBuffer *m_pConstants;
	uint4 m_constantBufferSize;
	lean::com_ptr<ID3D11Buffer> m_pConstantBuffer;

	uint4 m_unbufferedPropertiesBegin;
	uint4 m_unbufferedPropertiesEnd;
	property_vector m_properties;
	mutable bool m_bPropertiesChanged;
	
	texture_vector m_textures;

protected:
	/// Updates the given texture.
	void DependencyChanged(beGraphics::Texture *oldTexture, beGraphics::Texture *newTexture);

public:
	/// Constructor.
	BE_GRAPHICS_DX11_API Setup(const Effect *pEffect, beGraphics::TextureCache *pTextures);
	/// Copy constructor.
	BE_GRAPHICS_DX11_API Setup(const Setup &right);
	/// Destructor.
	BE_GRAPHICS_DX11_API ~Setup();

	/// Applys the setup.
	BE_GRAPHICS_DX11_API void Apply(const beGraphics::DeviceContext &context) const;

	/// Gets the number of properties.
	BE_GRAPHICS_DX11_API uint4 GetPropertyCount() const;
	/// Gets the ID of the given property.
	BE_GRAPHICS_DX11_API uint4 GetPropertyID(const utf8_ntri &name) const;
	/// Gets the name of the given property.
	BE_GRAPHICS_DX11_API utf8_ntr GetPropertyName(uint4 id) const;
	/// Gets the type of the given property.
	BE_GRAPHICS_DX11_API PropertyDesc GetPropertyDesc(uint4 id) const;

	/// Sets the given (raw) values.
	BE_GRAPHICS_DX11_API bool SetProperty(uint4 id, const std::type_info &type, const void *values, size_t count);
	/// Gets the given number of (raw) values.
	BE_GRAPHICS_DX11_API bool GetProperty(uint4 id, const std::type_info &type, void *values, size_t count) const;

	/// Visits a property for modification.
	BE_GRAPHICS_DX11_API bool WriteProperty(uint4 id, beCore::PropertyVisitor &visitor, bool bWriteOnly = true);
	/// Visits a property for reading.
	BE_GRAPHICS_DX11_API bool ReadProperty(uint4 id, beCore::PropertyVisitor &visitor, bool bPersistentOnly = false) const;

	/// Gets the number of textures.
	BE_GRAPHICS_DX11_API uint4 GetTextureCount() const;
	/// Gets the ID of the given texture.
	BE_GRAPHICS_DX11_API uint4 GetTextureID(const utf8_ntri &name) const;
	/// Gets the name of the given texture.
	BE_GRAPHICS_DX11_API utf8_ntr GetTextureName(uint4 id) const;
	/// Gets whether the texture is a color texture.
	BE_GRAPHICS_DX11_API bool IsColorTexture(uint4 id) const;

	/// Sets the given texture.
	BE_GRAPHICS_DX11_API void SetTexture(uint4 id, const beGraphics::TextureView *pView);
	/// Gets the given texture.
	BE_GRAPHICS_DX11_API const TextureView* GetTexture(uint4 id) const;

	/// Gets the number of child components.
	BE_GRAPHICS_DX11_API uint4 GetComponentCount() const;
	/// Gets the name of the n-th child component.
	BE_GRAPHICS_DX11_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_GRAPHICS_DX11_API const beCore::ReflectedComponent* GetReflectedComponent(uint4 idx) const;

	/// Gets the type of the n-th child component.
	BE_GRAPHICS_DX11_API beCore::Exchange::utf8_string GetComponentType(uint4 idx) const;
	/// Gets the n-th component.
	BE_GRAPHICS_DX11_API lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const;

	/// Returns true, if the n-th component can be replaced.
	BE_GRAPHICS_DX11_API bool IsComponentReplaceable(uint4 idx) const;
	/// Sets the n-th component.
	BE_GRAPHICS_DX11_API void SetComponent(uint4 idx, const lean::any &pComponent);

	/// Gets the effect.
	LEAN_INLINE const Effect* GetEffect() const { return m_pEffect; }

	/// Gets the type index.
	BE_GRAPHICS_DX11_API const beCore::TypeIndex* GetPropertyTypeIndex() const;

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return DX11Implementation; }
};

template <> struct ToImplementationDX11<beGraphics::Setup> { typedef Setup Type; };

} // namespace

} // namespace

#endif