/************************************************************/
/* breeze Engine Entity System Module  (c) Tobias Zirr 2011 */
/************************************************************/

#include "beEntitySystemInternal/stdafx.h"
#include "beEntitySystem/beRenderableHost.h"
#include <lean/functional/algorithm.h>
#include <lean/logging/errors.h>

namespace beEntitySystem
{

// Constructor.
RenderableHost::RenderableHost()
{
}

// Destructor.
RenderableHost::~RenderableHost()
{
}

// Renders all renderable content.
void RenderableHost::Render()
{
	for (renderable_vector::const_iterator it = m_render.begin(); it != m_render.end(); ++it)
		(*it)->Render();
}

// Adds a renderable controller.
void RenderableHost::AddRenderable(Renderable *pRenderable)
{
	if (!pRenderable)
	{
		LEAN_LOG_ERROR_MSG("pRenderable may not be nullptr");
		return;
	}

	m_render.push_back(pRenderable);
}

// Removes a renderable controller.
void RenderableHost::RemoveRenderable(Renderable *pRenderable)
{
	lean::remove(m_render, pRenderable);
}

} // namespace
