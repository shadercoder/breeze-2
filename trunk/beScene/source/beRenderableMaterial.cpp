/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderableMaterial.h"
#include "beScene/beAbstractRenderableEffectDriver.h"
#include "beScene/beEffectBinderCache.h"

namespace beScene
{

/// Technique.
struct RenderableMaterial::Technique
{
	beGraphics::Setup *pSetup;
	lean::resource_ptr<AbstractRenderableEffectDriver> pDriver;

	/// Constructor.
	Technique(beGraphics::Setup *pSetup,
		AbstractRenderableEffectDriver *pDriver)
			: pSetup(pSetup),
			pDriver(pDriver) { }
};

namespace
{

/// Loads techniques from the given material.
RenderableMaterial::technique_vector LoadTechniques(const Material &material, EffectBinderCache<AbstractRenderableEffectDriver> &driverCache)
{
	RenderableMaterial::technique_vector techniques;

	const uint4 techniqueCount = material.GetTechniqueCount();
	techniques.reset(techniqueCount);

	for (uint4 techniqueIdx = 0; techniqueIdx < techniqueCount; ++techniqueIdx)
	{
		AbstractRenderableEffectDriver *pEffectDriver = driverCache.GetEffectBinder(*material.GetTechnique(techniqueIdx));
		techniques.emplace_back(material.GetSetup(techniqueIdx), pEffectDriver);
	}

	return techniques;
}

} // namespace

// Constructor.
RenderableMaterial::RenderableMaterial(Material *pMaterial, EffectBinderCache<AbstractRenderableEffectDriver> &driverCache)
	: m_pMaterial( LEAN_ASSERT_NOT_NULL(pMaterial) ),
	m_techniques( LoadTechniques(*m_pMaterial, driverCache) )
{
}

// Destructor.
RenderableMaterial::~RenderableMaterial()
{
}

// Gets the number of techniques.
uint4 RenderableMaterial::GetTechniqueCount() const
{
	return static_cast<uint4>(m_techniques.size());
}

// Gets a technique by index.
RenderableMaterialTechnique RenderableMaterial::GetTechnique(uint4 techniqueIdx) const
{
	const Technique &technique = m_techniques[techniqueIdx];
	return RenderableMaterialTechnique(technique.pSetup, technique.pDriver);
}

} // namespace
