/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MATERIAL
#define BE_SCENE_MATERIAL

#include "beScene.h"
#include <beCore/beShared.h>
#include <beCore/beReflectedComponent.h>
#include <beGraphics/beEffect.h>
#include <beGraphics/beEffectCache.h>
#include <beGraphics/beTextureCache.h>
#include <beGraphics/beSetup.h>
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>
#include <vector>

namespace beScene
{

class MaterialCache;

/// Material class.
class Material : public lean::nonassignable, public beCore::OptionalPropertyProvider< beCore::RigidReflectedComponent<> >, public beCore::Resource
{
public:
	struct Technique;
	typedef std::vector<Technique> technique_vector;

	struct Setup;
	typedef std::vector<Setup> setup_vector;

private:
	lean::resource_ptr<const beGraphics::Effect> m_pEffect;
	
	technique_vector m_techniques;
	setup_vector m_setups;

	MaterialCache *m_pCache;

public:
	/// Constructor.
	BE_SCENE_API Material(const beGraphics::Effect *pEffect, beGraphics::EffectCache &effectCache, beGraphics::TextureCache &textureCache, MaterialCache *pCache = nullptr);
	/// Constructor.
	BE_SCENE_API Material(const Material &right);
	/// Destructor.
	BE_SCENE_API virtual ~Material();

	/// Gets the number of techniques.
	BE_SCENE_API uint4 GetTechniqueCount() const;
	/// Gets the setup for the given technique.
	BE_SCENE_API beGraphics::Setup* GetTechniqueSetup(uint4 techniqueIdx) const;
	/// Gets the technique identified by the given index.
	BE_SCENE_API const beGraphics::Technique* GetTechnique(uint4 techniqueIdx) const;
	/// Gets the name of the technique identified by the given index.
	BE_SCENE_API utf8_ntr GetTechniqueName(uint4 techniqueIdx) const;
	/// Gets the index of the technique identified by the given name.
	BE_SCENE_API uint4 GetTechniqueByName(const utf8_ntri &name) const;

	/// Gets the number of setups.
	BE_SCENE_API uint4 GetSetupCount() const;
	/// Gets the setup identified by the given index.
	BE_SCENE_API beGraphics::Setup* GetSetup(uint4 setupIdx) const;

	/// Gets the input signature of this pass.
	BE_SCENE_API const char* GetInputSignature(uint4 &size, uint4 techniqueIdx, uint4 passID = 0) const;

	/// Gets the number of child components.
	BE_SCENE_API uint4 GetComponentCount() const;
	/// Gets the name of the n-th child component.
	BE_SCENE_API beCore::Exchange::utf8_string GetComponentName(uint4 idx) const;
	/// Gets the n-th reflected child component, nullptr if not reflected.
	BE_SCENE_API const ReflectedComponent* GetReflectedComponent(uint4 idx) const;

	/// Gets the effect.
	LEAN_INLINE const beGraphics::Effect* GetEffect() const { return m_pEffect; }
	/// Gets the material cache.
	LEAN_INLINE MaterialCache* GetCache() const { return m_pCache; }
};

/// Transfers all data from the given source material to the given destination material.
BE_SCENE_API void Transfer(Material &dest, const Material &source);

} // namespace

#endif