/****************************************************/
/* breeze Engine Assets Module (c) Tobias Zirr 2013 */
/****************************************************/

#pragma once
#ifndef BE_ASSETS_STEPS
#define BE_ASSETS_STEPS

#include "beAssets.h"
#include <beEntitySystem/beEntityController.h>
#include <beEntitySystem/beEntities.h>
#include <beCore/beComponentObservation.h>

#include <lean/smart/resource_ptr.h>

#include <beMath/beMatrixDef.h>
#include <lean/containers/simple_vector.h>

namespace beAssets
{

/// Cover geometry class.
class StepsController : public beCore::PropertyFeedbackProvider< beEntitySystem::SingularEntityController >,
	public beCore::ComponentObserver
{
	struct Step;
	typedef lean::simple_vector<Step, lean::containers::vector_policies::semipod> steps_t;

private:
	steps_t m_steps;
	
	struct Config
	{
		uint4 stepCount;
		beMath::fvec3 linearDelta;
		beMath::fmat3 angularDelta;
		beMath::fvec3 scaleDelta;
		beMath::fvec3 deltaScaling;

		Config()
			: stepCount(1),
			angularDelta(beMath::fmat3::identity),
			scaleDelta(1.0f),
			deltaScaling(1.0f) { }
	} m_config;

	beEntitySystem::Entity *m_pEntity;

	void MaybeCommit() { if (m_pEntity) m_pEntity->NeedCommit(); }
	void MaybeSync() { if (m_pEntity) m_pEntity->NeedSync(); }

protected:
	/// Called when properties in the given provider might have changed.
	BE_ASSETS_API void PropertyChanged(const PropertyProvider &provider);

public:
	/// Constructor.
	BE_ASSETS_API StepsController();
	/// Constructor.
	BE_ASSETS_API StepsController(const StepsController &right);
	/// Destructor.
	BE_ASSETS_API ~StepsController();

	/// Sets the step count.
	void SetStepCount(uint4 count) { m_config.stepCount = count; MaybeCommit(); }
	/// Gets the step count.
	uint4 GetStepCount() const { return m_config.stepCount; }

	/// Sets the linear delta.
	void SetLinearDelta(const beMath::fvec3 &delta) { m_config.linearDelta = delta; MaybeSync(); EmitPropertyChanged(); }
	/// Gets the linear delta.
	beMath::fvec3 GetLinearDelta() const { return m_config.linearDelta; }

	/// Sets the angular delta.
	void SetAngularDelta(const beMath::fmat3 &delta) { m_config.angularDelta = delta; MaybeSync(); EmitPropertyChanged(); }
	/// Gets the angular delta.
	beMath::fmat3 GetAngularDelta() const { return m_config.angularDelta; }

	/// Sets the scale delta.
	void SetScaleDelta(const beMath::fvec3 &delta) { m_config.scaleDelta = delta; MaybeSync(); EmitPropertyChanged(); }
	/// Gets the scale delta.
	beMath::fvec3 GetScaleDelta() const { return m_config.scaleDelta; }

	/// Sets the scale delta.
	void SetDeltaScaling(const beMath::fvec3 &delta) { m_config.deltaScaling = delta; MaybeSync(); EmitPropertyChanged(); }
	/// Gets the scale delta.
	beMath::fvec3 GetDeltaScaling() const { return m_config.deltaScaling; }

	/// Synchronizes the scene with this controller.
	BE_ASSETS_API void Commit(beEntitySystem::EntityHandle entity);
	/// Synchronizes this controller with the controlled entity.
	BE_ASSETS_API void Synchronize(beEntitySystem::EntityHandle entity);

	/// Attaches this controller.
	BE_ASSETS_API void Attach(beEntitySystem::Entity *entity);
	/// Detaches this controller.
	BE_ASSETS_API void Detach(beEntitySystem::Entity *entity);

	/// Clones this entity controller.
	BE_ASSETS_API StepsController* Clone() const;
	
	/// Gets the reflection properties.
	BE_ASSETS_API static Properties GetOwnProperties();
	/// Gets the reflection properties.
	BE_ASSETS_API Properties GetReflectionProperties() const;

	/// Gets the controller type.
	BE_ASSETS_API static const beCore::ComponentType* GetComponentType();
	/// Gets the controller type.
	BE_ASSETS_API const beCore::ComponentType* GetType() const;
};

} // namespace

#endif