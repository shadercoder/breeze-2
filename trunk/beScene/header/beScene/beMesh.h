/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MESH
#define BE_SCENE_MESH

#include "beScene.h"
#include <beCore/beShared.h>
#include <beMath/beAABDef.h>
#include <beGraphics/beGraphics.h>

namespace beScene
{

class MeshCompound;

/// Mesh base.
class Mesh : public beCore::Resource, public beGraphics::Implementation
{
protected:
	MeshCompound *m_pCompound;		///< Compound.

	beMath::faab3 m_bounds;			///< Bounding box.

	LEAN_INLINE Mesh& operator =(const Mesh&) { return *this; }

public:
	/// Constructor.
	explicit LEAN_INLINE Mesh(MeshCompound *pCompound)
		: m_pCompound(pCompound) { }
	/// Constructor.
	LEAN_INLINE Mesh(const beMath::faab3& bounds, MeshCompound *pCompound)
		: m_pCompound(pCompound),
		m_bounds(bounds) { }
	/// Destructor.
	virtual ~Mesh() throw() { }

	/// Gets the bounding box.
	LEAN_INLINE const beMath::faab3& GetBounds() const { return m_bounds; } 

	/// Gets the mesh compound.
	LEAN_INLINE MeshCompound* GetCompound() const { return m_pCompound; }
};

} // namespace

#endif