/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PERSPECTIVE_MODIFIER
#define BE_SCENE_PERSPECTIVE_MODIFIER

#include "beScene.h"

namespace beScene
{

// Prototypes.
class RenderingPipeline;
struct PerspectiveDesc;

/// Perspective modifier interface.
class PerspectiveModifier
{
protected:
	PerspectiveModifier& operator =(const PerspectiveModifier&) { return *this; }
	~PerspectiveModifier() { }

public:
	/// Called when this modifier is about to be added to the given rendering pipeline.
	virtual void ModifierAdded(RenderingPipeline *pPipeline) { }
	/// Called when a perspective is about to be added. The given description may be modified.
	virtual void PerspectiveAdded(PerspectiveDesc &perspectiveDesc) = 0;
	/// Called this modifier has been returned from the given rendering pipeline.
	virtual void ModifierRemoved(RenderingPipeline *pPipeline) { }
};

/*

/// Chained perspective modifier base.
class ChainedPerspectiveModifier : public PerspectiveModifier
{
private:
	PerspectiveModifier *m_pPreviousModifier;

protected:
	ChainedPerspectiveModifier& operator =(const PerspectiveModifier&) { return *this; }
	
	/// Constructor.
	LEAN_INLINE ChainedPerspectiveModifier()
		: m_pPreviousModifier(nullptr) { }
	/// Destructor.
	LEAN_INLINE ~ChainedPerspectiveModifier()
	{
		ClearPreviousModifiers();
	}

	/// Removes all previous modifiers.
	LEAN_INLINE void ClearPreviousModifiers()
	{
		while (m_pPreviousModifier)
			m_pPreviousModifier = m_pPreviousModifier->ModifierRemoved(m_pPreviousModifier);
	}

public:
	/// Called when this modifier has been added. Return true if you intend to keep the previous modifier installed.
	virtual bool ModifierAdded(PerspectiveModifier *pPreviousModifier)
	{
		ClearPreviousModifiers();
		m_pPreviousModifier = pPreviousModifier;
		return true;
	}
	/// Called when the given modifier has been requested to be removed. An active chained modifier may be returned.
	virtual PerspectiveModifier* ModifierRemoved(PerspectiveModifier *pModifier)
	{
		if (pModifier == this)
			return m_pPreviousModifier;
		else if(m_pPreviousModifier)
			return m_pPreviousModifier->ModifierRemoved(pModifier);
		else
			return nullptr;
	}
};

*/

} // namespace

#endif