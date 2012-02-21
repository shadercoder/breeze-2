#define BE_RENDERABLE_INCLUDE_WORLD
#define BE_RENDERABLE_INCLUDE_PROJ

#include <Engine/Perspective.fx>
#include <Engine/Renderable.fx>
#include <Pipelines/LPR/Geometry.fx>

cbuffer SetupConstants
{
	float4 DiffuseColor
	<
		String UIName = "Diffuse";
	> = float4(1.0f, 1.0f, 1.0f, 0.03f);

	float4 SpecularColor
	<
		String UIName = "Specular";
	> = float4(1.0f, 1.0f, 1.0f, 0.0f);
}

struct Vertex
{
	float4 Position	: Position;
	float3 Normal	: Normal;
};

struct Pixel
{
	float4 Position		: SV_Position;
	float4 NormalDepth	: TexCoord0;
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.Position.z = 0.0f;
	o.NormalDepth.xyz = mul((float3x3) WorldInverse, v.Normal);
	o.NormalDepth.w = o.Position.w;
	
	return o;
}

GeometryOutput PSGeometry(Pixel p)
{
	return ReturnGeometry(p.NormalDepth.w, normalize(p.NormalDepth.xyz), DiffuseColor, SpecularColor);
}

technique11 Geometry <
	string PipelineStage = "GeometryPipelineStage";
	string RenderQueue = "DefaultRenderQueue";
>
{
	pass
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGeometry()) );
	}
}

technique11 <
	string IncludeEffect = "Prototypes/Feedback.fx";
> { }
