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
	float4 NormalDepth	: TexCoord0;
	float2 TexCoord		: TexCoord1;
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.NormalDepth.xyz = mul((float3x3) WorldInverse, v.Normal);
	o.NormalDepth.w = o.Position.w;
	o.TexCoord = v.TexCoord;
	
	return o;
}

SamplerState LinearSampler
{
	Filter = ANISOTROPIC;
	AddressU = WRAP;
	AddressV = WRAP;
};

GeometryOutput PSGeometry(Pixel p) // , uint primID : SV_PrimitiveID
{
	float3 normal = normalize(p.NormalDepth.xyz);
	float4 diffuse = DiffuseColor;
	diffuse.xyz *= FromSRGB(DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz);

//	diffuse = float4(
//		fmod(primID * 5333, 331) / 331, // fmod(primID * 134581, 13) / 13
//		fmod(primID * 7573, 307) / 307, // fmod(primID * 52361, 17) / 17
//		fmod(primID * 6733, 373) / 373, // fmod(primID * 331, 11) / 11
//		1.99999f );

//	if (p.NormalDepth.w != p.DepthC)
//	if (frac(p.Position.x) != 0.5f || frac(p.Position.y) != 0.5)
//		diffuse = float4(1, 0, 1, 1.9999f);

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
