#ifndef BE_MRGEO_H
#define BE_MRGEO_H

/// Scene geometry texture.
Texture2D SceneGeometryTexture2 : SceneGeometryTarget2
<
	string TargetType = "Permanent";
	string Format = "R16G16B16A16F";
>;

/// Scene geometry texture.
Texture2D SceneGeometryTexture4 : SceneGeometryTarget4
<
	string TargetType = "Permanent";
	string Format = "R16G16B16A16F";
>;

/// Scene geometry texture.
Texture2D SceneGeometryTexture8 : SceneGeometryTarget8
<
	string TargetType = "Permanent";
	string Format = "R16G16B16A16F";
>;

/// Scene geometry texture.
Texture2D SceneGeometryTexture16 : SceneGeometryTarget16
<
	string TargetType = "Permanent";
	string Format = "R16G16B16A16F";
>;

#endif