#ifndef BE_LPR_GEOMETRY_H
#define BE_LPR_GEOMETRY_H

struct GeometryOutput
{
	float4 DepthNormal	: SV_Target0;
	float4 Diffuse		: SV_Target1;
	float4 Specular		: SV_Target2;
};

GeometryOutput ReturnGeometry(float depth, float3 normal, float4 diffuse, float4 specular)
{
	GeometryOutput o;
	o.DepthNormal.x = depth;
	o.DepthNormal.yzw = normal;
	o.Diffuse = diffuse;
	o.Specular = specular;
	return o;
}

float4 BuildGeometry(float depth, float3 normal)
{
	float4 o;
	o.x = depth;
	o.yzw = normal;
	return o;
}

float ExtractDepth(float4 depthNormal)
{
	return depthNormal.x;
}

float3 ExtractNormal(float4 depthNormal)
{
	return depthNormal.yzw;
}

#endif