/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_DYNAMICSCENERY
#define BE_SCENE_DYNAMICSCENERY

#include "beScene.h"
#include "beScenery.h"
#include <lean/meta/type_traits.h>

namespace beScene
{

/// Renderable data allocator interface.
class RenderableDataAllocator
{
protected:
	RenderableDataAllocator& operator =(const RenderableDataAllocator&) { return *this; }
	~RenderableDataAllocator() { }

public:
	/// Allocates data from the shared renderable heap.
	virtual void* AllocateRenderableData(size_t size) = 0;
	/// Allocates data from the renderable pass heap.
	virtual void* AllocateRenderablePassData(size_t size) = 0;

	/// Allocates data from the shared renderable heap.
	template <class Type>
	LEAN_INLINE Type* AllocateRenderableData()
	{
		LEAN_STATIC_ASSERT_MSG_ALT(
			lean::is_trivially_destructible<Type>::value,
			"Renderable data must be of trivially destructible type.",
			Renderable_data_must_be_of_trivially_destructible_type);
		
		return static_cast<Type*>( AllocateRenderableData(sizeof(Type)) );
	}
	/// Allocates data from the renderable pass heap.
	template <class Type>
	LEAN_INLINE Type* AllocateRenderablePassData()
	{
		LEAN_STATIC_ASSERT_MSG_ALT(
			lean::is_trivially_destructible<Type>::value,
			"Renderable data must be of trivially destructible type.",
			Renderable_data_must_be_of_trivially_destructible_type);
		
		return static_cast<Type*>( AllocateRenderablePassData(sizeof(Type)) );
	}
};

/// Dynamic scenery interface.
class DynamicScenery : public Scenery
{
protected:
	DynamicScenery& operator =(const DynamicScenery&) { return *this; }
	~DynamicScenery() { }

public:
	/// Prepares the scenery for rendering access.
	virtual void Prepare() = 0;
	
	/// Adds the given renderable this scenery.
	virtual void AddRenderable(Renderable *pRenderable) = 0;
	/// Removes the given renderable from this scenery.
	virtual void RemoveRenderable(Renderable *pRenderable) = 0;
	/// Invalidates cached scenery data.
	virtual void InvalidateRenderables() = 0;

	/// Adds the given light to this scenery.
	virtual void AddLight(Light *pLight) = 0;
	/// Removes the given renderable from this scenery.
	virtual void RemoveLight(Light *pLight) = 0;
	/// Invalidates cached scenery data.
	virtual void InvalidateLights() = 0;
};

} // namespace

#endif