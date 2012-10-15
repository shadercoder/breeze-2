/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#ifndef BE_RESOURCECOMPILER_SCENEIMPL
#define BE_RESOURCECOMPILER_SCENEIMPL

#include "beResourceCompiler.h"
#include "beScene.h"
#include <assimp/scene.h>

namespace beResourceCompiler
{

/// Scene class.
class SceneImpl : public Scene
{
private:
	const aiScene *m_pScene;

public:
	/// Constructor.
	BE_RESOURCECOMPILER_API SceneImpl(const aiScene *pScene);
	/// Destructor.
	BE_RESOURCECOMPILER_API ~SceneImpl();

	/// Gets the scene.
	LEAN_INLINE const aiScene* GetScene() const { return m_pScene; }
};

}

#endif