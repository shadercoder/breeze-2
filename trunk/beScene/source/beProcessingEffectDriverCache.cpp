/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include "beSceneInternal/stdafx.h"
#include "beScene/beProcessingEffectDriverCache.h"
#include "beScene/beProcessingEffectDriver.h"
#include "beScene/beRenderingPipeline.h"
#include "beScene/bePerspectiveEffectBinderPool.h"
#include <beGraphics/Any/beEffect.h>
#include <unordered_map>

namespace beScene
{

/// Implementation.
class ProcessingEffectDriverCache::Impl
{
public:
	typedef std::unordered_map< ID3DX11EffectTechnique*, lean::resource_ptr<AbstractProcessingEffectDriver> > effect_driver_map;
	effect_driver_map effectDrivers;
};

// Constructor.
ProcessingEffectDriverCache::ProcessingEffectDriverCache(RenderingPipeline *pPipeline, PerspectiveEffectBinderPool *pPool)
	: m_impl( new Impl() ),
	m_pPipeline( pPipeline ),
	m_pPool( pPool )
{
}

// Destructor.
ProcessingEffectDriverCache::~ProcessingEffectDriverCache()
{
}

// Gets an effect binder from the given effect.
AbstractProcessingEffectDriver* ProcessingEffectDriverCache::GetEffectBinder(const beGraphics::Technique &technique, uint4 flags)
{
	ID3DX11EffectTechnique *pTechnique = ToImpl(technique);

	Impl::effect_driver_map::const_iterator it = m_impl->effectDrivers.find(pTechnique);

	if (it == m_impl->effectDrivers.end())
		it = m_impl->effectDrivers.insert( Impl::effect_driver_map::value_type(pTechnique, CreateEffectBinder(technique, flags)) ).first;

	return it->second;
}

// Creates an effect binder from the given effect.
lean::resource_ptr<AbstractProcessingEffectDriver, true> ProcessingEffectDriverCache::CreateEffectBinder(const beGraphics::Technique &technique, uint4 flags) const
{
	return lean::bind_resource<AbstractProcessingEffectDriver>( new ProcessingEffectDriver(technique, m_pPipeline, m_pPool) );
}

} // namespace