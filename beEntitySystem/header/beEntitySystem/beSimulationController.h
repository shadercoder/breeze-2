/************************************************************/
/* breeze Engine Simulation System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SIMULATIONCONTROLLER
#define BE_ENTITYSYSTEM_SIMULATIONCONTROLLER

#include "beEntitySystem.h"
#include "beController.h"
#include "beSimulation.h"
#include <lean/smart/weak_resource_ptr.h>

namespace beEntitySystem
{

/// Simulation controller base class.
class SimulationController : public Controller
{
protected:
	/// Controlled simulation.
	const lean::weak_resource_ptr<Simulation> m_pSimulation;

public:
	/// Constructor.
	LEAN_INLINE SimulationController(Simulation *pSimulation)
		: m_pSimulation( LEAN_ASSERT_NOT_NULL(pSimulation) ) { }

	/// Gets the controlled simulation.
	LEAN_INLINE Simulation* GetSimulation() const { return m_pSimulation.get_unchecked(); }
};

} // namespace

#endif