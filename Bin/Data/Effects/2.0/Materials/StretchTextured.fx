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
		String UIWidget = "Color";
	> = float4(1.0f, 1.0f, 1.0f, 0.03f);

	float4 SpecularColor
	<
		String UIName = "Specular";
		String UIWidget = "Color";
	> = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

Texture2D DiffuseTexture
<
	string UIName = "Diffuse";
>;

struct Vertex
{
	float4 Position	: Position;
	float3 Normal	: Normal;
	float2 TexCoord	: TexCoord;
};

struct Pixel
{
	float4 Position		: SV_Position;
	float4 World		: TexCoord0;
	float4 NormalDepth	: TexCoord1;
	float2 TexCoord		: TexCoord2;
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.World = mul(v.Position, World);
	o.NormalDepth.xyz = normalize( mul((float3x3) WorldInverse, v.Normal) );
	o.NormalDepth.w = o.Position.w;

	float3 scaledObj = v.Position * float3(length(World[0]), length(World[1]), length(World[2]));
	float3 mainDir = (abs(v.Normal) > 0.707);
	o.TexCoord.x = dot(mainDir, scaledObj.yzx);
	o.TexCoord.y = dot(mainDir, scaledObj.zxy);
	
	return o;
}

SamplerState LinearSampler
{
	Filter = ANISOTROPIC;
	AddressU = WRAP;
	AddressV = WRAP;
};

GeometryOutput PSGeometry(Pixel p)
{
	float3 normal = normalize(p.NormalDepth.xyz);
	float4 diffuse = DiffuseColor;
	diffuse.xyz *= DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz;
	return ReturnGeometry(p.NormalDepth.w, normal, diffuse, SpecularColor);
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
	string IncludeEffect = "Prototypes/Shadow.fx";
> { }

technique11 <
	string IncludeEffect = "Prototypes/Feedback.fx";
> { }
