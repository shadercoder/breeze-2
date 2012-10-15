/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#ifndef BE_RESOURCECOMPILER_MESHIMPL
#define BE_RESOURCECOMPILER_MESHIMPL

#include "beResourceCompiler.h"
#include "beMesh.h"
#include <assimp/scene.h>

namespace beResourceCompiler
{

/// Mesh class.
class MeshImpl : public Mesh
{
private:
	const aiScene *m_pScene;

public:
	/// Constructor.
	BE_RESOURCECOMPILER_API MeshImpl(const aiScene *pScene);
	/// Destructor.
	BE_RESOURCECOMPILER_API ~MeshImpl();

	/// Gets the scene.
	LEAN_INLINE const aiScene* GetScene() const { return m_pScene; }
};

}

#endif