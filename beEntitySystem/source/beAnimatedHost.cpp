/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beAnimatedHost.h"
#include <lean/functional/algorithm.h>
#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Constructor.
AnimatedHost::AnimatedHost()
{
}

// Destructor.
AnimatedHost::~AnimatedHost()
{
}

// Steps the simulation.
void AnimatedHost::Step(float timeStep)
{
	for (animated_vector::const_iterator it = m_animate.begin(); it != m_animate.end(); ++it)
		(*it)->Step(timeStep);
}

// Adds an animated controller.
void AnimatedHost::AddAnimated(Animated *pAnimated)
{
	if (!pAnimated)
	{
		LEAN_LOG_ERROR_MSG("pAnimated may not be nullptr");
		return;
	}

	m_animate.push_back(pAnimated);
}

// Removes an animated controller.
void AnimatedHost::RemoveAnimated(Animated *pAnimated)
{
	lean::remove(m_animate, pAnimated);
}

} // namespace
