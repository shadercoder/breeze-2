/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/DX11/beD3DXEffects11.h"
#include "beGraphics/DX11/beMaterial.h"
#include "beGraphics/DX11/beEffectConfig.h"
#include "beGraphics/DX11/beDeviceContext.h"
#include "beGraphics/DX11/beDevice.h"
#include "beGraphics/DX11/beEffect.h"

#include "beGraphics/beEffectCache.h"

#include "beGraphics/beTextureCache.h"
#include "beGraphics/DX11/beBuffer.h"

#include "beGraphics/DX/beError.h"

#include <beCore/beMany.h>

#include <beCore/bePropertyVisitor.h>

#include <lean/functional/algorithm.h>
#include <lean/logging/log.h>
#include <lean/io/numeric.h>

namespace beGraphics
{

namespace DX11
{

struct Material::DataSource
{
	lean::resource_ptr<beg::MaterialConfig> config;
	const MaterialConfigRevision *configRevision;
	MaterialConfigRevision materialRevision;
	
	uint4 constantBufferMask;
	bec::Range<uint4> properties;
	bec::Range<uint4> textures;

	DataSource(beg::MaterialConfig *config)
		: config(config),
		configRevision(config->GetRevision()) { }
};

struct Material::Constants
{
	API::EffectConstantBuffer *constants;
	lean::com_ptr<API::Buffer> buffer;
	uint4 offset;

	bec::Range<uint4> dataLinks;

	Constants(API::EffectConstantBuffer *constants, API::Buffer* buffer, uint4 offset, bec::Range<uint4> dataLinks)
		: constants(constants),
		buffer(buffer),
		offset(offset),
		dataLinks(dataLinks) { }
};

struct Material::ConstantDataLink
{
	uint4 srcOffset;
	uint4 srcLength;
	uint4 destOffset;

	ConstantDataLink(uint4 srcOffset, uint4 srcLength, uint4 destOffset)
		: srcOffset(srcOffset),
		srcLength(srcLength),
		destOffset(destOffset) { }
};

struct Material::Property
{
	utf8_string name;
	PropertyDesc desc;
	uint4 constantBufferMask;
	
	Property(utf8_ntri name,
		const PropertyDesc &desc,
		uint4 constantBufferMask = 0)
			: name(name.to<utf8_string>()),
			desc(desc),
			constantBufferMask(constantBufferMask) { }
};

struct Material::PropertyData
{
	uint4 offset;
	uint4 length;
	uint4 configIdx;

	PropertyData(uint4 offset, uint4 length)
		: offset(offset),
		length(length),
		configIdx(-1) { }
};

struct Material::Texture
{
	utf8_string name;
	bool bRaw;
	
	Texture(utf8_ntri name, bool bRaw)
		: name(name.to<utf8_string>()),
		bRaw(bRaw){ }
};

struct Material::TextureData
{
	lean::resource_ptr<const TextureView> pTexture;
	uint4 configIdx;

	TextureData()
		: configIdx(-1) { }
};

struct Material::TextureDataLink
{
	API::EffectShaderResource *variable;
	uint4 textureIdx;
	
	TextureDataLink(API::EffectShaderResource *variable, uint4 textureIdx)
		: variable(variable),
		textureIdx(textureIdx) { }
};

namespace
{

/// Technique.
struct InternalMaterialTechnique
{
	beg::Material *material;
	lean::resource_ptr<beg::Technique> technique;

	/// Constructor.
	InternalMaterialTechnique(Material *material,
			Technique *technique)
		: material(material),
		technique(technique) { }
};

LEAN_LAYOUT_COMPATIBLE(MaterialTechnique, Material, InternalMaterialTechnique, material);
LEAN_LAYOUT_COMPATIBLE(MaterialTechnique, Technique, InternalMaterialTechnique, technique);
LEAN_SIZE_COMPATIBLE(MaterialTechnique, InternalMaterialTechnique);

}

struct Material::Technique
{
	InternalMaterialTechnique internal;

	bec::Range<uint4> constants;
	bec::Range<uint4> textures;

	Technique(Material *material, DX11::Technique *technique,
			bec::Range<uint4> constants,
			bec::Range<uint4> textures)
		: internal(material, technique),
		constants(constants),
		textures(textures) { }
};

namespace
{

template <class Type>
struct NameSorter
{
	const Type &v;

	NameSorter(const Type &v)
		: v(v) { }

	template <class T>
	LEAN_INLINE const T& get(const T &val) const { return val; }
	LEAN_INLINE const utf8_string& get(uint4 idx) const { return v[idx].name; }

	template <class L, class R>
	LEAN_INLINE bool operator ()(const L &l, const R &r) const
	{
		return get(l) < get(r);
	}
};

/// Makes a sorted index list from the given range of named elements.
template <class Range>
void GenerateSortedIndices(Material::indices_t &sortedIndices, const Range &namedRange)
{
	sortedIndices.resize(namedRange.end() - namedRange.begin());
	std::generate(sortedIndices.begin(), sortedIndices.end(), lean::increment_gen<uint4>(0));
	std::sort(sortedIndices.begin(), sortedIndices.end(), NameSorter<Range>(namedRange));
}

// Gets the matching property.
uint4 FindMatchingProperty(const Material::properties_t &properties, const Material::indices_t &sortedProperties,
						   utf8_ntri name, const PropertyDesc &desc)
{
	const uint4 *sortedBegin = &sortedProperties[0], *sortedEnd = &sortedProperties[sortedProperties.size()];
	
	const uint4 *matchingBegin = std::lower_bound(sortedBegin, sortedEnd, name, NameSorter<Material::properties_t>(properties));
	const uint4 *matchingEnd = std::upper_bound(sortedBegin, sortedEnd, name, NameSorter<Material::properties_t>(properties));

	for (; matchingBegin < matchingEnd; ++matchingBegin)
	{
		const Material::Property &property = properties[*matchingBegin];

		if (property.desc.TypeDesc == desc.TypeDesc && property.desc.Count == desc.Count)
			return *matchingBegin;
	}

	return -1;
}

// Gets the matching texture.
uint4 FindMatchingTexture(const Material::textures_t &textures, const Material::indices_t &sortedTextures,
						   utf8_ntri name)
{
	const uint4 *sortedBegin = &sortedTextures[0], *sortedEnd = &sortedTextures[sortedTextures.size()];
	
	const uint4 *matching = lean::find_sorted(sortedBegin, sortedEnd, name, NameSorter<Material::textures_t>(textures));

	if (matching != sortedEnd)
		return *matching;

	return -1;
}

// Gets all properties.
bec::Range<uint4> GetProperties(API::Device *device, const EffectConfig &baseConfig, Material::Data &data, Material::indices_t &sortedProperties)
{
	bec::Range<uint4> relevantConstantRange = bec::MakeRangeN((uint4) data.constants.size(), 0U);

	const EffectConfig::ConstantBufferRange effectCBRange = baseConfig.GetConstantBufferInfo();
	const EffectConfig::ConstantRange effectConstantRange = baseConfig.GetConstantInfo();

	data.constants.reserve_grow_by(Size(baseConfig.GetConstantBufferInfo()));

	uint4 backingStoreOffset = static_cast<uint4>(data.backingStore.size());

	// Scan all constant buffers for properties
	for (const EffectConstantBufferInfo *itEffectCB = effectCBRange.Begin; itEffectCB < effectCBRange.End; ++itEffectCB)
	{
		uint4 cbufIdx = (uint4) data.constants.size();
		
		LEAN_ASSERT(cbufIdx < lean::size_info<uint4>::bits);
		uint4 cbufMask = 1 << cbufIdx;

		bec::Range<uint4> dataLinksRange;
		dataLinksRange.Begin = (uint4) data.constantDataLinks.size();

		for (uint4 effectPropertyIdx = itEffectCB->Constants.Begin; effectPropertyIdx < itEffectCB->Constants.End; ++effectPropertyIdx)
		{
			const EffectConstantInfo &constantInfo = effectConstantRange.Begin[effectPropertyIdx];
			uint4 destDataOffset = backingStoreOffset + constantInfo.BufferOffset;

			utf8_ntr propertyName = baseConfig.GetPropertyName(effectPropertyIdx);
			const bec::PropertyDesc &propertyDesc = baseConfig.GetPropertyDesc(effectPropertyIdx);
			uint4 propertyIdx = FindMatchingProperty(data.properties, sortedProperties, propertyName, propertyDesc);
			
			// No matching property
			if (propertyIdx == -1)
			{
				// Add new property
				propertyIdx = static_cast<uint4>( data.properties.size() );
				data.properties.push_back(
						Material::Property(propertyName, propertyDesc, cbufMask),
						Material::PropertyData(destDataOffset, propertyDesc.Count * propertyDesc.TypeDesc->Info.size)
					);
			}
			// Duplicate/spread matching property data
			else
			{
				// Store constant buffer reference
				data.properties[propertyIdx].constantBufferMask |= cbufMask;

				const Material::PropertyData &srcData = data.properties(Material::propertyData)[propertyIdx];
				bool bMergedDataLink = false;

				// Merge parallel source/destination constant data links
				if (!data.constantDataLinks.empty())
				{
					Material::ConstantDataLink &prevConst = data.constantDataLinks.back();

					if (prevConst.srcOffset + prevConst.srcLength == srcData.offset &&
						prevConst.destOffset + prevConst.srcLength == destDataOffset)
					{
						prevConst.srcLength += srcData.length;
						bMergedDataLink = true;
					}
				}

				// Insert new constant data link if none available for merging
				if (!bMergedDataLink)
					data.constantDataLinks.push_back(Material::ConstantDataLink(srcData.offset, srcData.length, destDataOffset));
			}

		}

		dataLinksRange.End = (uint4) data.constantDataLinks.size();

		// Add constant buffer
		data.constants.push_back(
				Material::Constants(
					itEffectCB->Variable,
					CreateConstantBuffer(device, itEffectCB->Size).get(),
					backingStoreOffset,
					dataLinksRange)
			);
		
		backingStoreOffset += itEffectCB->Size;
	}

	// Allocate bytes for backing store
	data.backingStore.resize(backingStoreOffset);

	// Sort properties by name
	GenerateSortedIndices(sortedProperties, data.properties);

	relevantConstantRange.End = (uint4) data.constants.size();
	return relevantConstantRange;
}

// Gets all textures.
bec::Range<uint4> GetTextures(const EffectConfig &baseConfig, Material::Data &data, Material::indices_t &sortedTextures)
{
	bec::Range<uint4> dataLinkRange = bec::MakeRangeN((uint4) data.textureDataLinks.size(), 0U);

	const EffectConfig::ResourceRange effectResourceRange = baseConfig.GetResourceInfo();

	// Scan all variables for textures
	for (const EffectResourceInfo *itEffectRes = effectResourceRange.Begin; itEffectRes < effectResourceRange.End; ++itEffectRes)
	{
		utf8_ntr textureName = baseConfig.GetTextureName(itEffectRes - effectResourceRange.Begin);
		uint4 textureIdx = FindMatchingTexture(data.textures, sortedTextures, textureName);

		// No matching texture
		if (textureIdx == -1)
		{
			// Add new texture
			textureIdx = (uint4) data.textures.size();
			data.textures.push_back(Material::Texture(textureName, itEffectRes->IsRaw));
		}

		data.textureDataLinks.push_back(Material::TextureDataLink(itEffectRes->Variable, textureIdx));
	}

	// Sort textures by name
	GenerateSortedIndices(sortedTextures, data.textures);

	dataLinkRange.End = (uint4) data.textureDataLinks.size();
	return dataLinkRange;
}

struct MaterialEffectVariables
{
	const Effect *effect;

	bec::Range<uint4> constants;
	bec::Range<uint4> textures;

	MaterialEffectVariables(const Effect *effect,
			bec::Range<uint4> constants,
			bec::Range<uint4> textures)
		: effect(effect),
		constants(constants), 
		textures(textures) { }
};


struct MaterialCreationContext
{
	typedef std::vector<MaterialEffectVariables> effect_vars_t;

	Material *const material;
	Material::Data *const data;
	effect_vars_t effectVars;
	Material::datasources_t *const dataSources;
	Material::indices_t sortedProperties, sortedTextures;

	MaterialCreationContext(Material *material, Material::Data *data, Material::datasources_t *dataSources)
		: material(material),
		data(data),
		dataSources(dataSources) { }
};

/// Adds the given effect to the given material.
MaterialEffectVariables AddEffectData(const Effect *effect, MaterialCreationContext &context)
{
	LEAN_ASSERT( effect );
	EffectConfig *defaultConfig = LEAN_ASSERT_NOT_NULL( effect->GetConfig() );

	for (MaterialCreationContext::effect_vars_t::const_iterator it = context.effectVars.begin(), itEnd = context.effectVars.end(); it < itEnd; ++it)
		if (it->effect == effect)
			return *it;

	lean::com_ptr<API::Device> device;
	effect->Get()->GetDevice(device.rebind());

	// Add effect
	context.effectVars.push_back(
			MaterialEffectVariables(
				effect,
				GetProperties(device, *defaultConfig, *context.data, context.sortedProperties),
				GetTextures(*defaultConfig, *context.data, context.sortedTextures)
			)
		);

	// Add default data
	context.dataSources->push_back( Material::DataSource(defaultConfig) );

	return context.effectVars.back();
}

/// Resets all bindings.
void ResetBindings(Material::properties_t &properties, Material::indices_t &propertiesByDS,
				   Material::textures_t &textures, Material::indices_t &texturesByDS)
{
	for (uint4 i = 0; i < properties.size(); ++i)
		properties(Material::propertyData)[i].configIdx = -1;
	propertiesByDS.clear();
	
	for (uint4 i = 0; i < textures.size(); ++i)
		textures(Material::textureData)[i].configIdx = -1;
	texturesByDS.clear();
}

/// Rebinds all properties.
void BindProperties(Material::DataSource &dataSource,
					Material::properties_t &properties, const Material::indices_t &sortedProperties, Material::indices_t &propertiesByDS)
{
	// Initialize data source binding
	dataSource.constantBufferMask = 0;
	dataSource.properties.Begin = (uint4) propertiesByDS.size();

	for (uint4 srcPropertyIdx = 0, srcPropertyCount = dataSource.config->GetPropertyCount();
		srcPropertyIdx < srcPropertyCount; ++srcPropertyIdx)
	{
		utf8_ntr propertyName = dataSource.config->GetPropertyName(srcPropertyIdx);
		const bec::PropertyDesc &propertyDesc = dataSource.config->GetPropertyDesc(srcPropertyIdx);
		uint4 propertyIdx = FindMatchingProperty(properties, sortedProperties, propertyName, propertyDesc);
		
		if (propertyIdx != -1)
		{
			Material::PropertyData &data = properties(Material::propertyData)[propertyIdx];

			// Check if property still unbound
			if (data.configIdx == -1)
			{
				// Bind property to data source
				data.configIdx = srcPropertyIdx;
				propertiesByDS.push_back(propertyIdx);

				// Bind constant buffer to data source
				dataSource.constantBufferMask |= properties[propertyIdx].constantBufferMask;
			}
		}
	}

	// Finalize data source binding range
	dataSource.properties.End = (uint4) propertiesByDS.size();
}

/// Rebinds all properties.
void BindTextures(Material::DataSource &dataSource,
				  Material::textures_t &textures, const Material::indices_t &sortedTextures, Material::indices_t &texturesByDS)
{
	// Initialize data source binding range
	dataSource.textures.Begin = (uint4) texturesByDS.size();

	for (uint4 srcTextureIdx = 0, srcTextureCount = dataSource.config->GetTextureCount();
		srcTextureIdx < srcTextureCount; ++srcTextureIdx)
	{
		utf8_ntr textureName = dataSource.config->GetTextureName(srcTextureIdx);
		uint4 textureIdx = FindMatchingTexture(textures, sortedTextures, textureName);
		
		if (textureIdx != -1)
		{
			Material::TextureData &data = textures(Material::textureData)[textureIdx];

			// Check if texture still unbound
			if (data.configIdx == -1)
			{
				// Bind texture to data source
				data.configIdx = srcTextureIdx;
				texturesByDS.push_back(textureIdx);
			}
		}
	}

	// Finalize data source binding range
	dataSource.textures.End = (uint4) texturesByDS.size();
}

void LoadTechniques(Material::techniques_t &techniques, MaterialCreationContext &creatctx, const MaterialEffectVariables &effectVars,
					beg::EffectCache &effectCache);

/// Adds the given technique.
void AddTechnique(Material::techniques_t &techniques, MaterialCreationContext &creatctx,
				  const MaterialEffectVariables &effectVars, Technique *effectTechnique, beg::EffectCache &effectCache)
{
	const char *includeEffect = "";

	// Check for & load effect cross-references
	if (SUCCEEDED(effectTechnique->Get()->GetAnnotationByName("IncludeEffect")->AsString()->GetString(&includeEffect)) )
	{
		std::vector<beGraphics::EffectMacro> includeMacros;
		std::vector<beGraphics::EffectHook> includeHooks;

		// Check for & load defines & hooks
		ID3DX11EffectStringVariable *pIncludeDefinitions = effectTechnique->Get()->GetAnnotationByName("IncludeDefinitions")->AsString();
		ID3DX11EffectStringVariable *pIncludeHooks = effectTechnique->Get()->GetAnnotationByName("IncludeHooks")->AsString();

		if (pIncludeDefinitions->IsValid())
		{
			typedef std::vector<const char*> string_pointers_t;
			string_pointers_t stringPtrs(beg::GetDesc(pIncludeDefinitions->GetType()).Elements);

			BE_THROW_DX_ERROR_MSG(
				pIncludeDefinitions->GetStringArray(&stringPtrs[0], 0, static_cast<UINT>(stringPtrs.size())),
				"ID3DX11EffectStringVariable::GetStringArray()" );
			includeMacros.reserve(stringPtrs.size());

			for (string_pointers_t::const_iterator it = stringPtrs.begin(); it != stringPtrs.end(); ++it)
			{
				utf8_ntr string(*it);
				// NOTE: Empty arrays unsupported, therefore check for empty dummy strings
				if (!string.empty())
					includeMacros.push_back( beGraphics::EffectMacro(string, utf8_ntr("")) );
			}
		}

		if (pIncludeHooks->IsValid())
		{
			typedef std::vector<const char*> string_pointers_t;
			string_pointers_t stringPtrs(beg::GetDesc(pIncludeHooks->GetType()).Elements);

			BE_THROW_DX_ERROR_MSG(
				pIncludeHooks->GetStringArray(&stringPtrs[0], 0, static_cast<UINT>(stringPtrs.size())),
				"ID3DX11EffectStringVariable::GetStringArray()" );
			includeHooks.reserve(stringPtrs.size());

			for (string_pointers_t::const_iterator it = stringPtrs.begin(); it != stringPtrs.end(); ++it)
			{
				utf8_ntr string(*it);
				// NOTE: Empty arrays unsupported, therefore check for empty dummy strings
				if (!string.empty())
					includeHooks.push_back( beGraphics::EffectHook(string) );
			}
		}

		// Load linked effect
		const Effect *linkedEffect = ToImpl(
				effectCache.GetByFile(includeEffect, &includeMacros[0], includeMacros.size(), &includeHooks[0], includeHooks.size())
			);
		MaterialEffectVariables linkedEffectVars = AddEffectData(linkedEffect, creatctx);

		const char *includeTechnique = "";

		// Check for single technique
		if (SUCCEEDED(effectTechnique->Get()->GetAnnotationByName("IncludeTechnique")->AsString()->GetString(&includeTechnique)))
		{
			ID3DX11EffectTechnique *pLinkedTechniqueDX = linkedEffect->Get()->GetTechniqueByName(includeTechnique);

			if (!pLinkedTechniqueDX->IsValid())
				LEAN_THROW_ERROR_CTX("Unable to locate technique specified in IncludeTechnique", includeTechnique);

			// Add single linked technique
			lean::resource_ptr<Technique> linkedTechnique = new_resource Technique(linkedEffect, pLinkedTechniqueDX);
			AddTechnique(techniques, creatctx, linkedEffectVars, linkedTechnique, effectCache);
		}
		else
			// Add all techniques
			LoadTechniques(techniques, creatctx, linkedEffectVars, effectCache);
	}
	else
		// Simply add the technique
		techniques.push_back(
				Material::Technique(
					creatctx.material, effectTechnique,
					effectVars.constants,
					effectVars.textures
				)
			);
}

/// Loads techniques & linked effects.
void LoadTechniques(Material::techniques_t &techniques, MaterialCreationContext &creatctx, const MaterialEffectVariables &effectVars,
					beGraphics::EffectCache &effectCache)
{
	api::Effect *effectDX = *effectVars.effect;
	D3DX11_EFFECT_DESC effectDesc = GetDesc(effectDX);

	techniques.reserve(effectDesc.Techniques);

	for (UINT id = 0; id < effectDesc.Techniques; ++id)
	{
		api::EffectTechnique *pTechniqueDX = effectDX->GetTechniqueByIndex(id);
		
		// Valid technique
		if (!pTechniqueDX->IsValid())
			LEAN_THROW_ERROR_MSG("ID3DX11Effect::GetTechniqueByIndex()");

		lean::resource_ptr<Technique> technique = new_resource Technique(effectVars.effect, pTechniqueDX);
		AddTechnique(techniques, creatctx, effectVars, technique, effectCache);
	}
}

} // namespace

// Constructor.
Material::Material(const beg::Effect *const* effects, uint4 effectCount, beg::EffectCache &effectCache)
{
	LEAN_ASSERT(effects && effectCount > 0);

	MaterialCreationContext creatctx(this, &m.data, &m.dataSources);

	for (uint4 effectIdx = 0; effectIdx < effectCount; ++effectIdx)
	{
		const Effect *effect = LEAN_ASSERT_NOT_NULL( ToImpl(effects[effectIdx]) );
		
		// Add as primary effect
		MaterialEffectVariables effectVars = AddEffectData(effect, creatctx);
		m.effects.push_back(effect);

		// Add techniques (and linked effects)
		LoadTechniques(m.techniques, creatctx, effectVars, effectCache);
	}

	m.immutableBaseConfigCount = (uint4) m.dataSources.size();
	
	m.hiddenEffectCount = 0;
	
	for (techniques_t::const_iterator it = m.techniques.begin(), itEnd = m.techniques.end(); it != itEnd; ++it)
	{
		bool bNew;
		lean::push_unique(m.effects, it->internal.technique->GetEffect(), &bNew);
		m.hiddenEffectCount += bNew;
	}
	
	// NOTE: Do lazily
	m.bBindingChanged = true;
//	Rebind();
}

// Copies the given material.
Material::Material(const Material &right) : m(right.m) { }

// Destructor.
Material::~Material()
{
}

// Rebinds the data sources.
void Material::Rebind()
{
	// Get rid of invalid bindings
	ResetBindings(m.data.properties, m.data.propertiesByDS, m.data.textures, m.data.texturesByDS);

	Material::indices_t sortedProperties, sortedTextures;
	GenerateSortedIndices(sortedProperties, m.data.properties);
	GenerateSortedIndices(sortedTextures, m.data.textures);

	for (datasources_t::iterator it = m.dataSources.begin(), itEnd = m.dataSources.end(); it < itEnd; ++it)
	{
		DataSource &dataSource = *it;

		// Rebind data sources
		BindProperties(dataSource, m.data.properties, sortedProperties, m.data.propertiesByDS);
		BindTextures(dataSource, m.data.textures, sortedTextures, m.data.texturesByDS);

		// Invalidate data, validate structure
		dataSource.materialRevision.Data = dataSource.configRevision->Data - 1;
		dataSource.materialRevision.Structure = dataSource.configRevision->Structure;
	}

	m.bBindingChanged = false;
}

// Applys the setup.
void Material::Apply(const MaterialTechnique *publicTechnique, const beGraphics::DeviceContext &context)
{
	bool bStructureChanged = m.bBindingChanged;
	bool bDataChanged = false;

	// TODO: Cache change detection via moving frame flag?
	for (uint4 dsIdx = 0, dsCount = (uint4) m.dataSources.size() - m.immutableBaseConfigCount; dsIdx < dsCount; ++dsIdx)
	{
		DataSource &dataSource = m.dataSources[dsIdx];
		
		// Check if bindings still up to date
		bStructureChanged |= (dataSource.materialRevision.Structure != dataSource.configRevision->Structure);
		bDataChanged |= (dataSource.materialRevision.Data != dataSource.configRevision->Data);
	}

	// Rebuild bindings, if outdated
	if (bStructureChanged)
	{
		Rebind();
		bDataChanged = true;
	}

	if (bDataChanged)
	{
		uint4 cbufChangedMask = 0;

		for (uint4 dsIdx = 0, dsCount = (uint4) m.dataSources.size(); dsIdx < dsCount; ++dsIdx)
		{
			DataSource &dataSource = m.dataSources[dsIdx];

			// Check if data still up to date
			if (dataSource.materialRevision.Data != dataSource.configRevision->Data)
			{
				// Update property data from data source
				for (bec::Range<uint4> dsProperties = dataSource.properties; dsProperties.Begin < dsProperties.End; ++dsProperties.Begin)
				{
					const PropertyData &data = m.data.properties(propertyData)[m.data.propertiesByDS[dsProperties.Begin]];
					ToImpl(*dataSource.config).GetPropertyRaw(data.configIdx, &m.data.backingStore[data.offset], data.length);
				}

				// Rebuild dependent constant buffers
				cbufChangedMask |= dataSource.constantBufferMask;

				// Update textures from data source
				for (bec::Range<uint4> dsTextures = dataSource.textures; dsTextures.Begin < dsTextures.End; ++dsTextures.Begin)
				{
					TextureData &data = m.data.textures(textureData)[m.data.texturesByDS[dsTextures.Begin]];
					data.pTexture = ToImpl(*dataSource.config).GetTexture(data.configIdx);
				}

				// Data up to date
				dataSource.materialRevision.Data = dataSource.configRevision->Data;
			}
		}

		// Bind constant buffers
		if (cbufChangedMask)
			for (uint4 cbufIdx = 0, cbufCount = (uint4) m.data.constants.size(); cbufIdx < cbufCount; ++cbufIdx)
				// Update changed constant buffers
				if (cbufChangedMask & (1 << cbufIdx))
				{
					const Constants &cbuf = m.data.constants[cbufIdx];

					// Spread updated property data
					for (bec::Range<uint4> dsLinks = cbuf.dataLinks; dsLinks.Begin < dsLinks.End; ++dsLinks.Begin)
					{
						const ConstantDataLink &link = m.data.constantDataLinks[dsLinks.Begin];
						memcpy(&m.data.backingStore[link.destOffset], &m.data.backingStore[link.srcOffset], link.srcLength);
					}

					ToImpl(context)->UpdateSubresource(cbuf.buffer, 0, nullptr, &m.data.backingStore[cbuf.offset], 0, 0);
				}
	}
	
	const Technique &technique = *reinterpret_cast<const Technique*>(publicTechnique);
	LEAN_ASSERT(m.techniques.begin() <= &technique && &technique < m.techniques.end());

	// Bind technique-related constant buffers
	for (uint4 cbufIdx = technique.constants.Begin; cbufIdx < technique.constants.End; ++cbufIdx)
	{
		// Bind constant buffer
		const Constants &cbuf = m.data.constants[cbufIdx];
		cbuf.constants->SetConstantBuffer(cbuf.buffer);
	}

	// Bind technique-related textures
	for (uint4 texDataIdx = technique.textures.Begin; texDataIdx < technique.textures.End; ++texDataIdx)
	{
		const TextureDataLink &link = m.data.textureDataLinks[texDataIdx];
		const TextureData &data = m.data.textures(textureData)[link.textureIdx];

		// Bind texture
		link.variable->SetResource( (data.pTexture) ? data.pTexture->GetView() : nullptr );
	}
}

/// Gets the effects.
Material::Effects Material::GetEffects() const
{
	return bec::MakeRangeN( &m.effects[0].get(), m.effects.size() - m.hiddenEffectCount );
}

/// Gets the effects.
Material::Effects Material::GetLinkedEffects() const
{
	return bec::MakeRangeN( &m.effects[0].get() + m.effects.size() - m.hiddenEffectCount, m.hiddenEffectCount );
}

// Gets the number of techniques.
uint4 Material::GetTechniqueCount() const
{
	return (uint4) m.techniques.size();
}

// Gets a technique by name.
uint4 Material::GetTechniqueIdx(const utf8_ntri &name)
{
	for (techniques_t::const_iterator it = m.techniques.begin(), itEnd = m.techniques.end(); it != itEnd; ++it)
		if (GetDesc(ToImpl(*it->internal.technique)).Name == name)
			return static_cast<uint4>(it - m.techniques.begin());

	return -1;
}

// Gets the number of techniques.
const MaterialTechnique* Material::GetTechnique(uint4 idx)
{
	return reinterpret_cast<const MaterialTechnique*>( &m.techniques[idx].internal );
}

// Gets the number of configurations.
uint4 Material::GetConfigurationCount() const
{
	return (uint4) m.dataSources.size() - m.immutableBaseConfigCount;
}

// Sets the given configuration.
void Material::SetConfiguration(uint4 idx, beg::MaterialConfig *config)
{
	LEAN_ASSERT(config);
	LEAN_ASSERT(idx < m.dataSources.size() - m.immutableBaseConfigCount);
	m.dataSources[idx] = DataSource( ToImpl(config) );
	m.bBindingChanged = true;
}

// Gets the configurations.
MaterialConfig* Material::GetConfiguration(uint4 idx) const
{
	LEAN_ASSERT(idx < m.dataSources.size() - m.immutableBaseConfigCount);
	return ToImpl(m.dataSources[idx].config.get());
}

// Sets all layered material configurations (important first).
void Material::SetConfigurations(beg::MaterialConfig *const *config, uint4 configCount)
{
	m.dataSources.erase(m.dataSources.begin(), m.dataSources.end() - m.immutableBaseConfigCount);
	m.dataSources.insert_disjoint(m.dataSources.begin(), config, config + configCount);
	m.bBindingChanged = true;
}

// Gets all layered material configurations (important first).
Material::Configurations Material::GetConfigurations() const
{
	return bec::MakeRangeN(
			Configurations::iterator(&m.dataSources[0].config.get(), sizeof(m.dataSources[0])),
			m.dataSources.size() - m.immutableBaseConfigCount
		);
}

struct Material::ReflectionBinding : public beCore::ResourceAsRefCounted< beCore::PropertyFeedbackProvider<MaterialReflectionBinding> >
{
	lean::resource_ptr<Material> material;

	struct Binding
	{
		lean::resource_ptr<MaterialConfig> source;
		uint4 sourceIdx;

		Binding()
			: source() { }
	};

	typedef lean::scoped_ptr<Binding[]> bindings_t;
	bindings_t propertySources;
	bindings_t textureSources;

	lean::resource_ptr<MaterialConfig> pTargetSource;

	ReflectionBinding(Material *material, MaterialConfig *pTargetSource)
		: material( LEAN_ASSERT_NOT_NULL(material) ),
		pTargetSource( pTargetSource )
	{
		const Material::Data &data = material->m.data;

		Material::indices_t sortedProperties, sortedTextures;
		GenerateSortedIndices(sortedProperties, data.properties);
		GenerateSortedIndices(sortedTextures, data.textures);

		propertySources = new Binding[data.properties.size()];
		textureSources = new Binding[data.textures.size()];

		MaterialConfig *pWaitForSource = pTargetSource;

		for (datasources_t::iterator it = material->m.dataSources.begin(), itEnd = material->m.dataSources.end(); it < itEnd; ++it)
		{
			const Material::DataSource &dataSource = *it;

			if (pWaitForSource && pWaitForSource != dataSource.config)
				continue;
			else
				pWaitForSource = nullptr;

			for (uint4 srcPropertyIdx = 0, srcPropertyCount = dataSource.config->GetPropertyCount();
				srcPropertyIdx < srcPropertyCount; ++srcPropertyIdx)
			{
				utf8_ntr propertyName = dataSource.config->GetPropertyName(srcPropertyIdx);
				const bec::PropertyDesc &propertyDesc = dataSource.config->GetPropertyDesc(srcPropertyIdx);
				uint4 propertyIdx = FindMatchingProperty(data.properties, sortedProperties, propertyName, propertyDesc);
				
				if (propertyIdx != -1)
				{
					Binding &binding = propertySources[propertyIdx];

					if (!binding.source)
					{
						binding.source = ToImpl(dataSource.config.get());
						binding.sourceIdx = srcPropertyIdx;
					}
				}
			}

			for (uint4 srcTextureIdx = 0, srcTextureCount = dataSource.config->GetTextureCount();
				srcTextureIdx < srcTextureCount; ++srcTextureIdx)
			{
				utf8_ntr textureName = dataSource.config->GetTextureName(srcTextureIdx);
				uint4 textureIdx = FindMatchingTexture(data.textures, sortedTextures, textureName);
		
				if (textureIdx != -1)
				{
					Binding &binding = textureSources[textureIdx];

					if (!binding.source)
					{
						binding.source = ToImpl(dataSource.config.get());
						binding.sourceIdx = srcTextureIdx;
					}
				}
			}
		}
	}

	/// Gets the number of properties.
	uint4 GetPropertyCount() const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		return (uint4) data.properties.size();
	}
	/// Gets the ID of the given property.
	uint4 GetPropertyID(const utf8_ntri &name) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		for (properties_t::const_iterator it = data.properties.begin(), itEnd = data.properties.end(); it != itEnd; ++it)
			if (it->name == name)
				return static_cast<uint4>(it - data.properties.begin());

		return static_cast<uint4>(-1);
	}
	/// Gets the name of the given property.
	utf8_ntr GetPropertyName(uint4 id) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());
		return utf8_ntr(data.properties[id].name);
	}
	/// Gets the type of the given property.
	PropertyDesc GetPropertyDesc(uint4 id) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());
		return data.properties[id].desc;
	}

	/// Adds and rebinds to the given property.
	void AddAndRebindProperty(uint4 id)
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());

		Binding &binding = propertySources[id];

		binding.sourceIdx = pTargetSource->GetPropertyID(data.properties[id].name);
		// TODO: Check desc?
		if (binding.sourceIdx == -1)
			// TODO: Move desc to effect config
			binding.sourceIdx = pTargetSource->AddProperty(data.properties[id].name, data.properties[id].desc);
		binding.source = pTargetSource;
	}

	/// Sets the given (raw) values.
	bool SetProperty(uint4 id, const std::type_info &type, const void *values, size_t count) LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());

		Binding &binding = propertySources[id];

		if (pTargetSource && binding.source != pTargetSource)
			AddAndRebindProperty(id);

		// TODO: Make effect configs immutable
		return binding.source->SetProperty(binding.sourceIdx, type, values, count);
	}
	/// Gets the given number of (raw) values.
	bool GetProperty(uint4 id, const std::type_info &type, void *values, size_t count) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());
		return propertySources[id].source->GetProperty(propertySources[id].sourceIdx, type, values, count);
	}

	/// Visits a property for modification.
	bool WriteProperty(uint4 id, beCore::PropertyVisitor &visitor, uint4 flags) LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());

		Binding &binding = propertySources[id];

		if (pTargetSource && binding.source != pTargetSource)
			AddAndRebindProperty(id);

		// TODO: Make effect configs immutable
		return binding.source->WriteProperty(binding.sourceIdx, visitor, flags);
	}
	/// Visits a property for reading.
	bool ReadProperty(uint4 id, beCore::PropertyVisitor &visitor,  uint4 flags) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.properties.size());
		return propertySources[id].source->ReadProperty(propertySources[id].sourceIdx, visitor, flags);
	}

	
	// Gets the number of textures.
	uint4 GetTextureCount() const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		return (uint4) data.textures.size();
	}

	// Gets the ID of the given texture.
	uint4 GetTextureID(const utf8_ntri &name) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		for (textures_t::const_iterator it = data.textures.begin(), itEnd = data.textures.end(); it != itEnd; ++it)
			if (it->name == name)
				return static_cast<uint4>(it - data.textures.begin());

		return static_cast<uint4>(-1);
	}
	
	/// Adds and rebinds to the given texture.
	void AddAndRebindTexture(uint4 idx)
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(idx < data.textures.size());

		Binding &binding = textureSources[idx];

		binding.sourceIdx = pTargetSource->GetTextureID(data.textures[idx].name);
		if (binding.sourceIdx == -1)
			binding.sourceIdx = pTargetSource->AddTexture(data.textures[idx].name, !data.textures[idx].bRaw);
		binding.source = pTargetSource;
	}

	// Gets the name of the given texture.
	utf8_ntr GetTextureName(uint4 id) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		return utf8_ntr(data.textures[id].name);
	}

	// Gets whether the texture is a color texture.
	bool IsColorTexture(uint4 id) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		return !data.textures[id].bRaw;
	}

	// Sets the given texture.
	void SetTexture(uint4 id, const beGraphics::TextureView *pView) LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.textures.size());

		Binding &binding = textureSources[id];

		if (pTargetSource && binding.source != pTargetSource)
			AddAndRebindTexture(id);

		// TODO: Make effect configs immutable
		binding.source->SetTexture(binding.sourceIdx, pView );
	}

	// Gets the given texture.
	const TextureView* GetTexture(uint4 id) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(id < data.textures.size());
		return textureSources[id].source->GetTexture(textureSources[id].sourceIdx);
	}


	/// Gets the number of child components.
	uint4 GetComponentCount() const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		return (uint4) data.textures.size();
	}
	/// Gets the name of the n-th child component.
	beCore::Exchange::utf8_string GetComponentName(uint4 idx) const LEAN_OVERRIDE
	{
		const Material::Data &data = material->m.data;
		LEAN_ASSERT(idx < data.textures.size());
		return lean::from_range<beCore::Exchange::utf8_string>( data.textures[idx].name );
	}
	/// Gets the n-th reflected child component, nullptr if not reflected.
	lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> GetReflectedComponent(uint4 idx) const LEAN_OVERRIDE
	{
		return nullptr;
	}

	/// Gets the type of the n-th child component.
	const beCore::ComponentType* GetComponentType(uint4 idx) const LEAN_OVERRIDE
	{
		return beg::TextureView::GetComponentType();
	}
	/// Gets the n-th component.
	lean::cloneable_obj<lean::any, true> GetComponent(uint4 idx) const LEAN_OVERRIDE
	{
		return bec::any_resource_t<beg::TextureView>::t( const_cast<beGraphics::DX11::TextureView*>( GetTexture(idx) ) );
	}

	/// Returns true, if the n-th component can be replaced.
	bool IsComponentReplaceable(uint4 idx) const LEAN_OVERRIDE
	{
		// TODO: Make effect configs immutable
		return true;
	}
	/// Sets the n-th component.
	void SetComponent(uint4 idx, const lean::any &pComponent) LEAN_OVERRIDE
	{
		SetTexture( idx, any_cast<beg::TextureView*>(pComponent) );
	}

	// Gets the type of the n-th child component.
	const beCore::ComponentType* GetType() const
	{
		static const beCore::ComponentType type = { "Material.ReflectionBinding" };
		return &type;
	}
};

// Gets a merged reflection binding.
lean::com_ptr<MaterialReflectionBinding, lean::critical_ref> Material::GetFixedBinding()
{
	return lean::bind_com( new ReflectionBinding(const_cast<Material*>(this), nullptr) );
}

// Gets a reflection binding for the given configuration.
lean::com_ptr<MaterialReflectionBinding, lean::critical_ref> Material::GetConfigBinding(uint4 configIdx)
{
	LEAN_ASSERT(configIdx < m.dataSources.size() - m.immutableBaseConfigCount);
	return lean::bind_com( new ReflectionBinding(this, ToImpl(m.dataSources[configIdx].config.get())) );
}

// Gets the number of child components.
uint4 Material::GetComponentCount() const
{
	return (uint4) m.dataSources.size() - m.immutableBaseConfigCount + 1;
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string Material::GetComponentName(uint4 idx) const
{
	beCore::Exchange::utf8_string name;

	LEAN_ASSERT(idx <= m.dataSources.size() - m.immutableBaseConfigCount);

	if (idx < m.dataSources.size() - m.immutableBaseConfigCount)
	{
		utf8_string num = lean::int_to_string(idx);
		name.reserve(lean::ntarraylen("Config ") + num.size());
		name.append("Config ");
		name.append(num.c_str(), num.c_str() + num.size());
	}
	else
		name = "Layered";

	return name;
}

// Returns true, if the n-th component is issential.
bool Material::IsComponentEssential(uint4 idx) const
{
	return idx == 0;
}

// Gets the n-th reflected child component, nullptr if not reflected.
lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> Material::GetReflectedComponent(uint4 idx) const
{
	LEAN_ASSERT(idx <= m.dataSources.size() - m.immutableBaseConfigCount);
	return (idx < m.dataSources.size() - m.immutableBaseConfigCount)
		? const_cast<Material*>(this)->GetConfigBinding(idx)
		: const_cast<Material*>(this)->GetFixedBinding();
}

// Gets the type of the n-th child component.
const beCore::ComponentType* Material::GetComponentType(uint4 idx) const
{
	return beg::MaterialConfig::GetComponentType();
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> Material::GetComponent(uint4 idx) const
{
	LEAN_ASSERT(idx <= m.dataSources.size() - m.immutableBaseConfigCount);
	return bec::any_resource_t<beg::MaterialConfig>::t(
			(idx < m.dataSources.size() - m.immutableBaseConfigCount) ? m.dataSources[idx].config : nullptr
		);
}

// Returns true, if the n-th component can be replaced.
bool Material::IsComponentReplaceable(uint4 idx) const
{
	return (idx < m.dataSources.size() - m.immutableBaseConfigCount);
}

// Sets the n-th component.
void Material::SetComponent(uint4 idx, const lean::any &pComponent)
{
	LEAN_ASSERT(idx <= m.dataSources.size() - m.immutableBaseConfigCount);
	// NOTE: Error-tolerant - actually, idx > 0 should be asserted
	if (idx < m.dataSources.size() - m.immutableBaseConfigCount)
		SetConfiguration(
				idx,
				// NOTE: Material configurations may not be unset
				LEAN_THROW_NULL(any_cast<beGraphics::MaterialConfig*>(pComponent))
			);
}

} // namespace

// Creates a new material.
lean::resource_ptr<Material, lean::critical_ref> CreateMaterial(const Effect *const* effects, uint4 effectCount, EffectCache &effectCache)
{
	return new_resource DX11::Material(effects, effectCount, effectCache);
}

// Creates a new material.
lean::resource_ptr<Material, lean::critical_ref> CreateMaterial(const Material &right)
{
	return new_resource DX11::Material(ToImpl(right));
}

} // namespace
