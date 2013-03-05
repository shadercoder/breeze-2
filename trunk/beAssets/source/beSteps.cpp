/****************************************************/
/* breeze Engine Assets Module (c) Tobias Zirr 2013 */
/****************************************************/

#include "beAssetsInternal/stdafx.h"
#include "beAssets/beSteps.h"

#include <beCore/beReflectionProperties.h>
#include <beCore/beSpecialReflectionProperties.h>

#include <beEntitySystem/beGenericControllerSerializer.h>
#include <beEntitySystem/beSerialization.h>

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

namespace beAssets
{

BE_CORE_PUBLISH_COMPONENT(StepsController)

const beCore::ReflectionProperty ControllerProperties[] =
{
	bec::MakeReflectionProperty<uint4>("step count", bec::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&StepsController::SetStepCount) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&StepsController::GetStepCount) ),
	bec::MakeReflectionProperty<float[3]>("delta", bec::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&StepsController::SetLinearDelta, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&StepsController::GetLinearDelta, float) ),
	bec::MakeReflectionProperty<float[3]>("angles (zxy)", bec::Widget::Angle)
		.set_setter( bec::MakeEulerZXYMatrixSetter( BE_CORE_PROPERTY_SETTER_UNION(&StepsController::SetAngularDelta, float) ) )
		.set_getter( bec::MakeEulerZXYMatrixGetter( BE_CORE_PROPERTY_GETTER_UNION(&StepsController::GetAngularDelta, float) ) ),
	bec::MakeReflectionProperty<float[3]>("scale delta", bec::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&StepsController::SetScaleDelta, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&StepsController::GetScaleDelta, float) ),
	bec::MakeReflectionProperty<float[3]>("delta scaling", bec::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER_UNION(&StepsController::SetDeltaScaling, float) )
		.set_getter( BE_CORE_PROPERTY_GETTER_UNION(&StepsController::GetDeltaScaling, float) )
};
BE_CORE_ASSOCIATE_PROPERTIES(StepsController, ControllerProperties)
		
const bees::EntityControllerSerializationPlugin< bees::GenericControllerSerializer<StepsController> > ControllerSerializerPlugin;

struct StepsController::Step
{
	lean::scoped_ptr<bees::Entity> entity;

	Step(bees::Entity *entity)
		: entity(entity) { }
};

// Constructor.
StepsController::StepsController()
	: m_pEntity(nullptr)
{
}

// Constructor.
StepsController::StepsController(const StepsController &right)
	: m_pEntity(nullptr),
	m_config(right.m_config)
{
}

// Destructor.
StepsController::~StepsController()
{
}

// Called when properties in the given provider might have changed.
void StepsController::PropertyChanged(const bec::PropertyProvider &provider)
{
	if (m_pEntity && !m_steps.empty() && &provider == m_steps[0].entity)
	{
		bees::Entity *entity1 = m_pEntity;
		bees::Entity *entity2 = m_steps[0].entity;

		bem::fvec3 linearDelta = mul( entity1->GetOrientation(), entity2->GetPosition() - entity1->GetPosition() );
		bem::fmat3 angularDelta = mul( entity2->GetOrientation(), transpose(entity1->GetOrientation()) );
		bem::fvec3 scaleDelta = entity2->GetScaling() / entity1->GetScaling();

		static const float eps = 0.0001f;

		if (!eps_eq(linearDelta, m_config.linearDelta, eps) |
			!eps_eq(angularDelta, m_config.angularDelta, eps) ||
			!eps_eq(scaleDelta, m_config.scaleDelta, eps))
		{
			m_config.linearDelta = linearDelta;
			m_config.angularDelta = angularDelta;
			m_config.scaleDelta = scaleDelta;
			MaybeSync();
		}
	}
}

// Synchronizes the scene with this controller.
void StepsController::Commit(beEntitySystem::EntityHandle entity)
{
	// NOTE: Controlled entity == first step
	uint4 additionalStepCount = (m_config.stepCount > 0) ? m_config.stepCount - 1 : 0;

	// _Remove_ old entities
	if (m_steps.size() > additionalStepCount)
		m_steps.shrink(additionalStepCount);
	// Add missing entities
	else
	{
		// IMPORTANT: Don't clone this controller
		struct Filter : bees::EntityControllerFilter
		{
			StepsController *self;

			Filter(StepsController *self)
				: self(self) { }

			bool Accept(const bees::EntityController *controller) LEAN_OVERRIDE
			{
				return controller != self;
			}
		} filter(this);

		// Create new steps
		for (uint4 stepIdx = (uint4) m_steps.size(); stepIdx < additionalStepCount; ++stepIdx)
			new_emplace(m_steps) Step( entity.Group->CloneEntity(entity, bees::Entities::AnonymousPersistentID, &filter) );
	}

	// Place steps
	bees::Entities::NeedSync(entity);
}

// Synchronizes this controller with the controlled entity.
void StepsController::Synchronize(beEntitySystem::EntityHandle entity)
{
	using bees::Entities;
	Entities::Transformation trafo = Entities::GetTransformation(entity);

	bem::fvec3 linDelta = m_config.linearDelta;

	for (uint4 stepIdx = 0, stepCount = (uint4) m_steps.size(); stepIdx < stepCount; ++stepIdx)
	{
		trafo.Position += mul(linDelta, trafo.Orientation);
		trafo.Orientation = mul(trafo.Orientation, m_config.angularDelta);
		trafo.Scaling *= m_config.scaleDelta;
		linDelta *= m_config.deltaScaling;

		Entities::SetTransformation(m_steps[stepIdx].entity->Handle(), trafo);
	}

	if (!m_steps.empty())
		m_steps[0].entity->AddObserver(this);
}

// Attaches this controller.
void StepsController::Attach(beEntitySystem::Entity *entity)
{
	LEAN_ASSERT(!m_pEntity);
	m_pEntity = entity;

	entity->NeedCommit();
}

// Detaches this controller.
void StepsController::Detach(beEntitySystem::Entity *entity)
{
	LEAN_ASSERT(entity == m_pEntity);
	m_pEntity = nullptr;
}

/// Clones this entity controller.
StepsController* StepsController::Clone() const
{
	return new StepsController(*this);
}

} // namespace