/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beMaterial.h"
#include <beGraphics/Any/beEffect.h>
#include <beGraphics/Any/beSetup.h>
#include <beGraphics/DX/beError.h>

#include <lean/io/filesystem.h>
#include <lean/io/numeric.h>

namespace beScene
{

/// Technique.
struct Material::Technique
{
	lean::resource_ptr<const beGraphics::Any::Technique> technique;
	lean::resource_ptr<beGraphics::Any::Setup> setup;

	/// Constructor.
	Technique(const beGraphics::Any::Technique *pTechnique,
		beGraphics::Any::Setup *pSetup)
			: technique(pTechnique),
			setup(pSetup) { }
};

/// Setup.
struct Material::Setup
{
	lean::resource_ptr<beGraphics::Any::Setup> setup;

	/// Constructor.
	explicit Setup(beGraphics::Any::Setup *pSetup)
		: setup(pSetup) { }
};

namespace
{

void LoadTechniques(Material::technique_vector &techniques, Material::setup_vector &setups,
	const beGraphics::Any::Effect *pEffect, beGraphics::Any::Setup *pSetup,
	beGraphics::EffectCache &effectCache, beGraphics::TextureCache &textureCache);

/// Gets a setup for the given effect.
beGraphics::Any::Setup* GetSetup(Material::setup_vector &setups, const beGraphics::Any::Effect *pEffect, beGraphics::TextureCache &textureCache)
{
	for (Material::setup_vector::const_iterator it = setups.begin(); it != setups.end(); ++it)
		// Try to find matching setup
		if (pEffect == it->setup->GetEffect())
			return it->setup;

	// Create new setup for material, if non existent, yet
	lean::resource_ptr<beGraphics::Any::Setup> pSetup = lean::new_resource<beGraphics::Any::Setup>(pEffect, &textureCache);
	setups.emplace_back(pSetup);
	return pSetup;
}

/// Adds the given technique.
void AddTechnique(Material::technique_vector &techniques, Material::setup_vector &setups, const beGraphics::Any::Effect *pEffect, beGraphics::Any::Setup *pSetup,
	ID3DX11EffectTechnique *pTechniqueDX, beGraphics::EffectCache &effectCache, beGraphics::TextureCache &textureCache)
{
	const char *includeEffect = "";

	// Check for & load effect cross-references
	if (SUCCEEDED(pTechniqueDX->GetAnnotationByName("IncludeEffect")->AsString()->GetString(&includeEffect)) )
	{
		std::vector<beGraphics::EffectMacro> includeMacros;

		// Check for & load defines
		ID3DX11EffectStringVariable *pIncludeDefinitions = pTechniqueDX->GetAnnotationByName("IncludeDefinitions")->AsString();

		if (pIncludeDefinitions->IsValid())
		{
			D3DX11_EFFECT_TYPE_DESC includeDefinitionDesc;

			if (BE_LOG_DX_ERROR_MSG(
					pIncludeDefinitions->GetType()->GetDesc(&includeDefinitionDesc),
					"ID3DX11EffectVariable::GetDesc()" ))
			{
				typedef std::vector<const char*> macro_pointer_vector;
				macro_pointer_vector macroPointers(includeDefinitionDesc.Elements);
				
				if (BE_LOG_DX_ERROR_MSG(
						pIncludeDefinitions->GetStringArray(&macroPointers[0], 0, static_cast<UINT>(macroPointers.size())),
						"ID3DX11EffectStringVariable::GetStringArray()" ))
				{
					includeMacros.reserve(macroPointers.size());

					for (macro_pointer_vector::const_iterator it = macroPointers.begin(); it != macroPointers.end(); ++it)
						includeMacros.push_back( beGraphics::EffectMacro(utf8_ntr(*it), utf8_ntr("")) );
				}
			}
		}

		// Load nested effect
		const beGraphics::Any::Effect *pNestedEffect = ToImpl( effectCache.GetEffect(includeEffect, &includeMacros[0], includeMacros.size()) );
		
		beGraphics::Any::Setup *pNestedSetup = GetSetup(setups, pNestedEffect, textureCache);

		const char *includeTechnique = "";

		// Check for single technique
		if (SUCCEEDED(pTechniqueDX->GetAnnotationByName("IncludeTechnique")->AsString()->GetString(&includeTechnique)))
		{
			ID3DX11EffectTechnique *pNestedTechniqueDX = pNestedEffect->Get()->GetTechniqueByName(includeTechnique);

			if (!pNestedTechniqueDX->IsValid())
				LEAN_THROW_ERROR_CTX("ID3DX11Effect::GetTechniqueByName()", includeTechnique);

			// Add single technique
			AddTechnique(techniques, setups, pNestedEffect, pNestedSetup, pNestedTechniqueDX, effectCache, textureCache);
		}
		else
			// Add all techniques
			LoadTechniques(techniques, setups, pNestedEffect, pNestedSetup, effectCache, textureCache);
	}
	else
		// Simply add the technique
		techniques.push_back(
				Material::Technique(
					lean::new_resource<beGraphics::Any::Technique>(pEffect, pTechniqueDX).get(),
					pSetup
				)
			);
}

/// Loads techniques from the given effect.
void LoadTechniques(Material::technique_vector &techniques, Material::setup_vector &setups,
	const beGraphics::Any::Effect *pEffect, beGraphics::Any::Setup *pSetup,
	beGraphics::EffectCache &effectCache, beGraphics::TextureCache &textureCache)
{
	ID3DX11Effect *pEffectDX = *pEffect;
	D3DX11_EFFECT_DESC effectDesc = beGraphics::DX11::GetDesc(pEffectDX);

	techniques.reserve(effectDesc.Techniques);

	for (UINT id = 0; id < effectDesc.Techniques; ++id)
	{
		ID3DX11EffectTechnique *pTechniqueDX = pEffectDX->GetTechniqueByIndex(id);
		
		// Valid technique
		if (!pTechniqueDX->IsValid())
			LEAN_THROW_ERROR_MSG("ID3DX11Effect::GetTechniqueByIndex()");

		AddTechnique(techniques, setups, pEffect, pSetup, pTechniqueDX, effectCache, textureCache);
	}
}

/// Clones the given techniques.
void CloneTechniques(Material::technique_vector &techniques, Material::setup_vector &setups,
	const Material::technique_vector &sourceTechniques, const Material::setup_vector &sourceSetups)
{
	setups.reserve(sourceSetups.size());

	// Clone setups first
	for (Material::setup_vector::const_iterator it = sourceSetups.begin(); it != sourceSetups.end(); ++it)
		setups.emplace_back( lean::new_resource<beGraphics::Any::Setup>(*it->setup).get() );

	techniques.reserve(sourceTechniques.size());

	for (Material::technique_vector::const_iterator itTechnique = sourceTechniques.begin(); itTechnique != sourceTechniques.end(); ++itTechnique)
	{
		beGraphics::Any::Setup *clonedSetup = nullptr;

		// Find cloned setup
		for (Material::setup_vector::const_iterator it = sourceSetups.begin(); it != sourceSetups.end(); ++it)
			if (itTechnique->setup == it->setup)
				clonedSetup = setups[it - sourceSetups.begin()].setup;

		LEAN_ASSERT(clonedSetup != nullptr);

		// Add technique using cloned setup
		techniques.push_back(
				Material::Technique(
					itTechnique->technique,
					clonedSetup
				)
			);
	}
	
}

} // namespace

// Constructor.
Material::Material(const beGraphics::Effect *pEffect, beGraphics::EffectCache &effectCache, beGraphics::TextureCache &textureCache, MaterialCache *pCache)
	: m_pEffect(pEffect),
	m_pCache(pCache)
{
	LoadTechniques(
			m_techniques, m_setups,
			&ToImpl(*m_pEffect), beScene::GetSetup(m_setups, &ToImpl(*m_pEffect), textureCache),
			effectCache, textureCache
		);
}

// Constructor.
Material::Material(const Material &right)
	: m_pEffect(right.m_pEffect),
	m_pCache(right.m_pCache)
{
	CloneTechniques(m_techniques, m_setups, right.m_techniques, right.m_setups);
}

// Destructor.
Material::~Material()
{
}

// Gets the number of techniques.
uint4 Material::GetTechniqueCount() const
{
	return static_cast<uint4>(m_techniques.size());
}

// Gets the setup for the given technique.
beGraphics::Setup* Material::GetTechniqueSetup(uint4 techniqueIdx) const
{
	return (techniqueIdx < m_techniques.size())
		? m_techniques[techniqueIdx].setup
		: nullptr;
}

// Gets the technique identified by the given index.
const beGraphics::Technique* Material::GetTechnique(uint4 techniqueIdx) const
{
	return (techniqueIdx < m_techniques.size())
		? m_techniques[techniqueIdx].technique
		: nullptr;
}

// Gets the name of the technique identified by the given index.
utf8_ntr Material::GetTechniqueName(uint4 techniqueIdx) const
{
	utf8_ntr name("");

	if (techniqueIdx < m_techniques.size())
	{
		D3DX11_TECHNIQUE_DESC desc;
		
		if (BE_LOG_DX_ERROR_MSG(
				m_techniques[techniqueIdx].technique->Get()->GetDesc(&desc),
				"ID3DX11EffectTechnique::GetDesc()"
			) )
			name = utf8_ntr(desc.Name);
	}

	return name;
}

// Gets the index of the technique identified by the given name.
uint4 Material::GetTechniqueByName(const utf8_ntri &name) const
{
	for (uint4 i = 0; i < m_techniques.size(); ++i)
	{
		D3DX11_TECHNIQUE_DESC desc;

		if (BE_LOG_DX_ERROR_MSG(
				m_techniques[i].technique->Get()->GetDesc(&desc),
				"ID3DX11EffectTechnique::GetDesc()"
			) && desc.Name == name)
				return i;
	}

	return static_cast<uint4>(-1);
}

// Gets the number of setups.
uint4 Material::GetSetupCount() const
{
	return static_cast<uint4>(m_setups.size());
}

// Gets the setup identified by the given index.
beGraphics::Setup* Material::GetSetup(uint4 setupIdx) const
{
	return (setupIdx < m_setups.size())
		? m_setups[setupIdx].setup
		: nullptr;
}

// Gets the input signature of this pass.
const char* Material::GetInputSignature(uint4 &size, uint4 techniqueIdx, uint4 passID) const
{
	D3DX11_PASS_DESC passDesc;

	BE_THROW_DX_ERROR_MSG(
		m_techniques[techniqueIdx].technique->Get()->GetPassByIndex(passID)->GetDesc(&passDesc),
		"ID3DX11EffectPass::GetDesc()" );

	size = static_cast<uint4>(passDesc.IAInputSignatureSize);
	return reinterpret_cast<const char*>(passDesc.pIAInputSignature);
}

// Gets the number of child components.
uint4 Material::GetComponentCount() const
{
	return static_cast<uint4>( m_setups.size() );
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string Material::GetComponentName(uint4 idx) const
{
	beCore::Exchange::utf8_string name;

	if (idx < m_setups.size())
	{
		const beGraphics::Effect *pEffect = m_setups[idx].setup->GetEffect();
		beGraphics::EffectCache *pCache = pEffect->GetCache();

		if (pCache)
			name = lean::get_stem<beCore::Exchange::utf8_string>( pCache->GetFile(*pEffect) );
	}

	// Ugly default names
	if (name.empty())
	{
		utf8_string num = lean::int_to_string(idx);
		name.reserve(lean::ntarraylen("Setup ") + num.size());
		name.append("Setup ");
		name.append(num.c_str(), num.c_str() + num.size());
	}
	
	return name;
}

// Gets the n-th reflected child component, nullptr if not reflected.
const beCore::ReflectedComponent* Material::GetReflectedComponent(uint4 idx) const
{
	return (idx < m_setups.size())
		? m_setups[idx].setup
		: nullptr;
}

// Transfers all data from the given source material to the given destination material.
void Transfer(Material &dest, const Material &source)
{
	const uint4 count = dest.GetSetupCount();
	const uint4 techniqueCount = dest.GetTechniqueCount();
	const uint4 srcCount = source.GetSetupCount();
	const uint4 srcTechniqueCount = source.GetTechniqueCount();

	const beGraphics::Setup &srcMainSetup = *source.GetSetup(0);

	for (uint4 setupIdx = 0; setupIdx < count; ++setupIdx)
	{
		beGraphics::Setup &setup = *dest.GetSetup(setupIdx);
		const beGraphics::Effect *effect = setup.GetEffect();
		const beGraphics::EffectCache *pCache = effect->GetCache();
		
		bool bTransferred = false;

		// Transfer by effect
		for (uint4 srcSetupIdx = 0; srcSetupIdx < srcCount; ++srcSetupIdx)
		{
			const beGraphics::Setup &srcSetup = *source.GetSetup(srcSetupIdx);
			const beGraphics::Effect *srcEffect = srcSetup.GetEffect();

			if (effect == srcEffect || pCache && pCache->Equivalent(*effect, *srcEffect))
			{
				beGraphics::Transfer(setup, srcSetup);
				bTransferred = true;
				break;
			}
		}

		if (bTransferred)
			continue;

		if (pCache)
			// Transfer by effect file
			for (uint4 srcSetupIdx = 0; srcSetupIdx < srcCount; ++srcSetupIdx)
			{
				const beGraphics::Setup &srcSetup = *source.GetSetup(srcSetupIdx);
				const beGraphics::Effect *srcEffect = srcSetup.GetEffect();

				if (pCache->Equivalent(*effect, *srcEffect, true))
				{
					beGraphics::Transfer(setup, srcSetup);
					bTransferred = true;
					break;
				}
			}

		if (bTransferred)
			continue;
		
		// Transfer by name
		for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
		{
			if (&setup == dest.GetTechniqueSetup(techniqueIdx))
			{
				utf8_ntr techniqueName = dest.GetTechniqueName(techniqueIdx);

				for (uint4 srcTechniqueIdx = 0; srcTechniqueIdx < srcTechniqueCount; ++srcTechniqueIdx)
					if (techniqueName == source.GetTechniqueName(srcTechniqueIdx))
					{
						beGraphics::Transfer(setup, *source.GetTechniqueSetup(srcTechniqueIdx));
						bTransferred = true;
						break;
					}

				if (bTransferred)
					break;
			}
		}

		// Transfer main
		if (!bTransferred)
			beGraphics::Transfer(setup, srcMainSetup);
	}
}

} // namespace
