/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beRenderableEffectDriverCache.h"
#include "beScene/beRenderableEffectDriver.h"
#include "beScene/beRenderableProcessingEffectDriver.h"
#include "beScene/beRenderingPipeline.h"
#include <beGraphics/Any/beEffect.h>
#include <unordered_map>

namespace beScene
{

/// Implementation.
class RenderableEffectDriverCache::Impl
{
public:
	typedef std::unordered_map< ID3DX11EffectTechnique*, lean::resource_ptr<AbstractRenderableEffectDriver> > effect_driver_map;
	effect_driver_map effectDrivers;
};

// Constructor.
RenderableEffectDriverCache::RenderableEffectDriverCache(RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool)
	: m_impl( new Impl() ),
	m_pPipeline(pPipeline),
	m_pPool(pPool)
{
}

// Destructor.
RenderableEffectDriverCache::~RenderableEffectDriverCache()
{
}

// Gets an effect binder from the given effect.
AbstractRenderableEffectDriver* RenderableEffectDriverCache::GetEffectBinder(const beGraphics::Technique &technique, uint4 flags)
{
	ID3DX11EffectTechnique *pTechnique = ToImpl(technique);

	Impl::effect_driver_map::const_iterator it = m_impl->effectDrivers.find(pTechnique);

	if (it == m_impl->effectDrivers.end())
		it = m_impl->effectDrivers.insert( Impl::effect_driver_map::value_type(pTechnique, CreateEffectBinder(technique, flags)) ).first;

	return it->second;
}

// Creates an effect binder from the given effect.
lean::resource_ptr<AbstractRenderableEffectDriver, true> RenderableEffectDriverCache::CreateEffectBinder(const beGraphics::Technique &technique, uint4 flags) const
{
	AbstractRenderableEffectDriver *pDriver;

	BOOL requiresProcessing = false;
	ToImpl(technique)->GetAnnotationByName("EnableProcessing")->AsScalar()->GetBool(&requiresProcessing);

	if (requiresProcessing)
		pDriver = new RenderableProcessingEffectDriver(technique, m_pPipeline, m_pPool, flags);
	else
		pDriver = new RenderableEffectDriver(technique, m_pPipeline, m_pPool, flags);

	return lean::bind_resource(pDriver);
}

} // namespace