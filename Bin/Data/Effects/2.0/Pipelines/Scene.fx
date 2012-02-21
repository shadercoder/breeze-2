#ifndef BE_SCENE_H
#define BE_SCENE_H

#include "Engine/BindPoints.fx"

#ifdef BE_SCENE_SETUP
	#define BE_SCENE_TEXTURE(name) name
#else
	#define BE_SCENE_TEXTURE(name) prebound(name)
#endif

/// Scene texture.
Texture2D BE_SCENE_TEXTURE(SceneTexture) : bindpoint_s(SceneTarget, t15)
<
	string TargetType = "Permanent";
	string Format = "R16G16B16A16F";
>;

#endif