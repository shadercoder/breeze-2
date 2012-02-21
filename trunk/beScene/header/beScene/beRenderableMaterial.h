/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_RENDERABLE_MATERIAL
#define BE_SCENE_RENDERABLE_MATERIAL

#include "beScene.h"
#include "beMaterial.h"
#include <beCore/beShared.h>
#include <beGraphics/beEffect.h>
#include <beGraphics/beSetup.h>
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>
#include <lean/containers/dynamic_array.h>

namespace beScene
{

// Prototypes
class AbstractRenderableEffectDriver;
template <class EffectBinder>
class EffectBinderCache;

/// Material layer.
struct RenderableMaterialTechnique
{
	beGraphics::Setup *Setup;							/// Material properties.
	const AbstractRenderableEffectDriver *EffectDriver;	///< Effect driver.

	/// Constructor.
	RenderableMaterialTechnique(beGraphics::Setup *pSetup,
		const AbstractRenderableEffectDriver *pDriver)
			: Setup(pSetup),
			EffectDriver(pDriver) { }
};

/// Material class.
class RenderableMaterial : public lean::nonassignable, public beCore::Resource
{
public:
	struct Technique;
	typedef lean::dynamic_array<Technique> technique_vector;

private:
	lean::resource_ptr<Material> m_pMaterial;
	
	technique_vector m_techniques;

public:
	/// Constructor.
	BE_SCENE_API RenderableMaterial(Material *pMaterial, EffectBinderCache<AbstractRenderableEffectDriver> &driverCache);
	/// Destructor.
	BE_SCENE_API virtual ~RenderableMaterial();

	/// Gets the number of techniques.
	BE_SCENE_API uint4 GetTechniqueCount() const;
	/// Gets a technique by index.
	BE_SCENE_API RenderableMaterialTechnique GetTechnique(uint4 techniqueIdx) const;

	/// Gets the material.
	LEAN_INLINE Material* GetMaterial() { return m_pMaterial; } 
	/// Gets the material.
	LEAN_INLINE const Material* GetMaterial() const { return m_pMaterial; } 
};

} // namespace

#endif