#ifndef BE_LPR_SCENE_H
#define BE_LPR_SCENE_H

#include "Pipelines/Scene.fx"
#include "Engine/BindPoints.fx"

/// Scene depth texture.
Texture2D BE_SCENE_TEXTURE(SceneGeometryTexture) : bindpoint_s(SceneGeometryTarget, t14)
<
	string TargetType = "Permanent";
	string Format = "R16G16B16A16F";
>;

/// Scene texture.
Texture2D BE_SCENE_TEXTURE(SceneDiffuseTexture) : bindpoint_s(SceneDiffuseTarget, t13)
<
	string TargetType = "Permanent";
	string Format = "R8G8B8A8U_SRGB";
>;

/// Scene texture.
Texture2D BE_SCENE_TEXTURE(SceneSpecularTexture) : bindpoint_s(SceneSpecularTarget, t12)
<
	string TargetType = "Permanent";
	string Format = "R8G8B8A8U_SRGB";
>;

/// Depth buffer.
Texture2D BE_SCENE_TEXTURE(SceneDepthBuffer) : SceneDepthBuffer
<
	string TargetType = "Permanent";
	string Format = "D24S8";
>;

#endif