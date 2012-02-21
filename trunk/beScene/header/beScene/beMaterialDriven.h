/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_MATERIAL_DRIVEN
#define BE_SCENE_MATERIAL_DRIVEN

#include "beScene.h"
#include "beMaterial.h"
#include <lean/smart/resource_ptr.h>

namespace beScene
{

// Prototypes.
class Material;

/// Material-driven base.
class MaterialDriven
{
protected:
	lean::resource_ptr<Material> m_pMaterial;	///< Material.

	MaterialDriven& operator =(const MaterialDriven&) { return *this; }
	~MaterialDriven() { }

public:
	/// Constructor.
	MaterialDriven(Material *pMaterial)
		: m_pMaterial(pMaterial) { }

	/// Sets the material of this renderable object.
//	virtual void SetMaterial(Material *pMaterial) = 0; // Omit V-Table

	/// Gets the material of this renderable object.
	LEAN_INLINE Material* GetMaterial() const { return m_pMaterial; };
};

} // namespace

#endif