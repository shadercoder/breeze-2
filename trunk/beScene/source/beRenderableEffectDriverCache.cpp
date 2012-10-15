/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderableEffectDriverCache.h"
#include "beScene/beRenderableEffectDriver.h"
#include "beScene/beRenderableProcessingEffectDriver.h"
#include "beScene/beRenderingPipeline.h"
#include <beGraphics/Any/beEffect.h>
#include <beGraphics/Any/beEffectsAPI.h>
#include <unordered_map>

namespace beScene
{

/// Implementation.
class RenderableEffectDriverCache::M
{
public:
	typedef std::unordered_map< ID3DX11EffectTechnique*, lean::resource_ptr<AbstractRenderableEffectDriver> > effect_driver_map;
	effect_driver_map effectDrivers;
};

// Constructor.
RenderableEffectDriverCache::RenderableEffectDriverCache(RenderingPipeline *pipeline, PerspectiveEffectBinderPool *pool)
	: m( new M() ),
	m_pipeline( LEAN_ASSERT_NOT_NULL(pipeline) ),
	m_pool( LEAN_ASSERT_NOT_NULL(pool) )
{
}

// Destructor.
RenderableEffectDriverCache::~RenderableEffectDriverCache()
{
}

// Gets an effect binder from the given effect.
AbstractRenderableEffectDriver* RenderableEffectDriverCache::GetEffectBinder(const beGraphics::Technique &technique, uint4 flags)
{
	LEAN_PIMPL_M

	beg::api::EffectTechnique *techniqueDX = ToImpl(technique);

	M::effect_driver_map::const_iterator it = m.effectDrivers.find(techniqueDX);

	if (it == m.effectDrivers.end())
		it = m.effectDrivers.insert( M::effect_driver_map::value_type(techniqueDX, CreateEffectBinder(technique, flags)) ).first;

	return it->second;
}

// Creates an effect binder from the given effect.
lean::resource_ptr<AbstractRenderableEffectDriver, true> RenderableEffectDriverCache::CreateEffectBinder(const beGraphics::Technique &technique, uint4 flags) const
{
	AbstractRenderableEffectDriver *driver;

	BOOL requiresProcessing = false;
	ToImpl(technique)->GetAnnotationByName("EnableProcessing")->AsScalar()->GetBool(&requiresProcessing);

	if (requiresProcessing)
		driver = new RenderableProcessingEffectDriver(technique, m_pipeline, m_pool, flags);
	else
		driver = new RenderableEffectDriver(technique, m_pipeline, m_pool, flags);

	return lean::bind_resource(driver);
}

} // namespace