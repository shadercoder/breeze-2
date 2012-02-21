/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#ifndef BE_ENTITYSYSTEM_SIMULATION
#define BE_ENTITYSYSTEM_SIMULATION

#include "beEntitySystem.h"
#include <beCore/beShared.h>
#include "beControllerDriven.h"
#include "beSynchronizedHost.h"
#include "beAnimatedHost.h"
#include "beRenderableHost.h"
#include <beCore/beExchangeContainers.h>

namespace beEntitySystem
{

/// Simulation class.
class Simulation : public beCore::Resource, public ControllerDriven,
	public SynchronizedHost, public AnimatedHost, public RenderableHost
{
private:
	utf8_string m_name;

protected:
	Simulation& operator =(const Simulation&) { return *this; }

public:
	/// Constructor.
	BE_ENTITYSYSTEM_API Simulation(const utf8_ntri &name);
	/// Destructor.
	BE_ENTITYSYSTEM_API virtual ~Simulation();

	/// Sets the name.
	BE_ENTITYSYSTEM_API void SetName(const utf8_ntri &name);
	/// Gets the name.
	LEAN_INLINE const utf8_string& GetName() const { return m_name; }

	/// Gets the simulation type.
	BE_ENTITYSYSTEM_API virtual beCore::Exchange::utf8_string GetType() const;
};

} // namespace

#endif