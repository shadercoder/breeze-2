/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/beRigidShapeCache.h"

#include <beCore/beResourceIndex.h>
#include <beCore/beResourceManagerImpl.hpp>

#include "bePhysics/beAssembledShape.h"
#include "bePhysics/beMaterial.h"
#include "bePhysics/beDevice.h"

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace bePhysics
{

/// Shape cache implementation
struct RigidShapeCache::M
{
	RigidShapeCache *cache;
	lean::resource_ptr<Device> device;
	
	struct Info
	{
		lean::resource_ptr<RigidShape> resource;

		Info(RigidShape *resource)
			: resource(resource) { }
	};

	typedef bec::ResourceIndex<RigidShape, Info> resources_t;
	resources_t resourceIndex;

	lean::resource_ptr<Material> pDefaultMaterial;

	lean::resource_ptr<beCore::ComponentMonitor> pComponentMonitor;

	/// Constructor.
	M(RigidShapeCache *cache, Device *device)
		: cache(cache),
		device(device) { }
};

// Constructor.
RigidShapeCache::RigidShapeCache(Device *device)
	: m( new M(this, device) )
{
}

// Destructor.
RigidShapeCache::~RigidShapeCache()
{
}

// Commits / reacts to changes.
void RigidShapeCache::Commit()
{
	LEAN_PIMPL();

	if (!m.pComponentMonitor ||
		!m.pComponentMonitor->Replacement.HasChanged(Material::GetComponentType()) &&
		!m.pComponentMonitor->Replacement.HasChanged(AssembledShape::GetComponentType()))
		return;

	bool bMeshHasChanges = false;
	bool bDataHasChanges = false;

	for (M::resources_t::iterator it = m.resourceIndex.Begin(), itEnd = m.resourceIndex.End(); it != itEnd; ++it)
	{
		RigidShape *shape = it->resource;

		const AssembledShape *oldSource = shape->GetSource();
		const AssembledShape *newSource = bec::GetSuccessor(shape->GetSource());

		if (newSource != oldSource)
		{
			lean::resource_ptr<RigidShape> newShape = ToRigidShape(*newSource, nullptr, true);
			if (!TransferMaterials(*shape, *newShape))
				FillRigidShape(*newShape, GetDefaultMaterial());

			Replace(shape, newShape);
			shape = newShape;
			bMeshHasChanges = true;
		}

		RigidShape::SubsetRange subsets = shape->GetSubsets();
		for (uint4 i = 0, count = Size4(subsets); i < count; ++i)
		{
			Material *material = bec::GetSuccessor(subsets[i].Material.get());
			if (material != subsets[i].Material)
			{
				shape->SetMaterial(i, material);
				bDataHasChanges = true;
			}
		}
	}

	// Notify dependents
	if (bMeshHasChanges)
		m.pComponentMonitor->Replacement.AddChanged(RigidShape::GetComponentType());
	if (bDataHasChanges)
		m.pComponentMonitor->Data.AddChanged(RigidShape::GetComponentType());
}

// Sets a default material.
void RigidShapeCache::SetDefaultMaterial(Material *defaultMaterial)
{
	m->pDefaultMaterial = defaultMaterial;
}

// Gets a default material.
Material* RigidShapeCache::GetDefaultMaterial()
{
	LEAN_PIMPL();

	if (!m.pDefaultMaterial)
		m.pDefaultMaterial = CreateMaterial(*m.device, 1.0f, 1.0f, 0.0f);

	return m.pDefaultMaterial;
}

// Sets the component monitor.
void RigidShapeCache::SetComponentMonitor(beCore::ComponentMonitor *componentMonitor)
{
	m->pComponentMonitor = componentMonitor;
}

// Gets the component monitor.
beCore::ComponentMonitor* RigidShapeCache::GetComponentMonitor() const
{
	return m->pComponentMonitor;
}

// Creates a new shape cache.
lean::resource_ptr<RigidShapeCache, true> CreateRigidShapeCache(Device *device)
{
	return new_resource RigidShapeCache(device);
}

} // namespace