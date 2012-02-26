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

void LoadTechniques(Material::technique_vector &techniques, Material::setup_vector &setups, const beGraphics::Effect *pEffect, beGraphics::EffectCache &effectCache);

/// Adds the given technique.
void AddTechnique(Material::technique_vector &techniques, Material::setup_vector &setups, const beGraphics::Effect *pEffect, beGraphics::Any::Setup *pSetup,
	ID3DX11EffectTechnique *pTechniqueDX, beGraphics::EffectCache &effectCache)
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
		const beGraphics::Effect *pNestedEffect = effectCache.GetEffect(includeEffect, &includeMacros[0], includeMacros.size());

		const char *includeTechnique = "";

		// Check for single technique
		if (SUCCEEDED(pTechniqueDX->GetAnnotationByName("IncludeTechnique")->AsString()->GetString(&includeTechnique)))
		{
			ID3DX11EffectTechnique *pNestedTechniqueDX = ToImpl(*pNestedEffect)->GetTechniqueByName(includeTechnique);

			if (!pNestedTechniqueDX->IsValid())
				LEAN_THROW_ERROR_CTX("ID3DX11Effect::GetTechniqueByName()", includeTechnique);

			lean::resource_ptr<beGraphics::Any::Setup> pNestedSetup;

			for (Material::technique_vector::const_iterator it = techniques.begin(); it != techniques.end(); ++it) // TODO: Material sharing partly broken
				// Try to find matching setup
				if (pNestedEffect == it->technique->GetEffect())
				{
					pNestedSetup = it->setup;
					break;
				}

			// Create new setup for material, if non existent, yet
			if (!pNestedSetup)
			{
				pNestedSetup = lean::new_resource<beGraphics::Any::Setup>( &ToImpl(*pNestedEffect) );
				setups.emplace_back( pNestedSetup );
			}

			// Add single technique
			AddTechnique(techniques, setups, pNestedEffect, pNestedSetup, pNestedTechniqueDX, effectCache);
		}
		else
			// Add all techniques
			LoadTechniques(techniques, setups, pNestedEffect, effectCache); // TODO: Material sharing partly broken
	}
	else
		// Simply add the technique
		techniques.push_back(
				Material::Technique(
					lean::new_resource<beGraphics::Any::Technique>(&ToImpl(*pEffect), pTechniqueDX).get(),
					pSetup
				)
			);
}

/// Loads techniques from the given effect.
void LoadTechniques(Material::technique_vector &techniques, Material::setup_vector &setups, const beGraphics::Effect *pEffect, beGraphics::EffectCache &effectCache)
{
	ID3DX11Effect *pEffectDX = ToImpl(*pEffect);

	// Create new setup for material
	lean::resource_ptr<beGraphics::Any::Setup> pSetup = lean::bind_resource(
			new beGraphics::Any::Setup( &ToImpl(*pEffect) ) // TODO: Material sharing partly broken
		);
	setups.emplace_back( pSetup );

	D3DX11_EFFECT_DESC effectDesc;
	BE_THROW_DX_ERROR_MSG(pEffectDX->GetDesc(&effectDesc), "ID3DX11Effect::GetDesc()");

	techniques.reserve(effectDesc.Techniques);

	for (UINT id = 0; id < effectDesc.Techniques; ++id)
	{
		ID3DX11EffectTechnique *pTechniqueDX = pEffectDX->GetTechniqueByIndex(id);
		
		// Valid technique
		if (!pTechniqueDX->IsValid())
			LEAN_THROW_ERROR_MSG("ID3DX11Effect::GetTechniqueByIndex()");

		AddTechnique(techniques, setups, pEffect, pSetup, pTechniqueDX, effectCache);
	}
}

} // namespace

// Constructor.
Material::Material(const beGraphics::Effect *pEffect, beGraphics::EffectCache &effectCache, MaterialCache *pCache)
	: m_pEffect(pEffect),
	m_pCache(pCache)
{
	LoadTechniques(m_techniques, m_setups, m_pEffect, effectCache);
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
beGraphics::Setup* Material::GetSetup(uint4 techniqueIdx) const
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

} // namespace
