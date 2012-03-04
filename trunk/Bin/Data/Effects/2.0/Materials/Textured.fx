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
	> = float4(0.05f, 0.05f, 0.05f, 0.9f);

	float2 TextureRepeat
	<
		String UIName = "Texture Repeat";
	> = float2(1.0f, 1.0f);
}

Texture2D DiffuseTexture
<
	string UIName = "Diffuse";
>;

#ifdef NORMALMAP

	Texture2D NormalTexture
	<
		string UIName = "Normal";
	>;

	#define IF_NORMALMAP(x) x

#else
	#define IF_NORMALMAP(x)
#endif

#ifdef SPECULARMAP

	Texture2D SpecularTexture
	<
		string UIName = "Specular";
	>;

	#define IF_SPECULARMAP(x) x

#else
	#define IF_SPECULARMAP(x)
#endif

struct Vertex
{
	float4 Position	: Position;
	float3 Normal	: Normal;
	float2 TexCoord	: TexCoord;

	IF_NORMALMAP(
		float3 Tangent	: Tangent;
	)
};

struct Pixel
{
	float4 Position		: SV_Position;
	float4 NormalDepth	: TexCoord0;
	float2 TexCoord		: TexCoord1;
	
	IF_NORMALMAP(
		float3 Tangent		: TexCoord2;
	)
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.NormalDepth.xyz = mul((float3x3) WorldInverse, v.Normal);
	o.NormalDepth.w = o.Position.w;
	o.TexCoord = v.TexCoord * TextureRepeat;

	IF_NORMALMAP(
		o.Tangent = mul(v.Tangent, (float3x3) World);
	)
	
	return o;
}

SamplerState LinearSampler
{
	Filter = ANISOTROPIC;
	AddressU = WRAP;
	AddressV = WRAP;
};

float3 DecodeNormal(float3 n)
{
	n.xy = n.xy * 2.0f - 1.0f;
	return n;
}

GeometryOutput PSGeometry(Pixel p) // , uint primID : SV_PrimitiveID
{
	float3 normal = p.NormalDepth.xyz;
	
	IF_NORMALMAP(
		float3 tangent = p.Tangent;
		float3 bitangent = cross(normal, tangent);
		
		normal = mul( DecodeNormal(NormalTexture.Sample(LinearSampler, p.TexCoord).xyz), float3x3(tangent, bitangent, normal) );
	)
	
	normal = normalize(normal);

	float4 diffuse = DiffuseColor;
	diffuse.xyz *= FromSRGB(DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz);

	float4 specular = SpecularColor;
	IF_SPECULARMAP(
		specular.xyz *= SpecularTexture.Sample(LinearSampler, p.TexCoord).xyz;
	)

//	diffuse = float4(
//		fmod(primID * 5333, 331) / 331, // fmod(primID * 134581, 13) / 13
//		fmod(primID * 7573, 307) / 307, // fmod(primID * 52361, 17) / 17
//		fmod(primID * 6733, 373) / 373, // fmod(primID * 331, 11) / 11
//		1.99999f );

//	if (p.NormalDepth.w != p.DepthC)
//	if (frac(p.Position.x) != 0.5f || frac(p.Position.y) != 0.5)
//		diffuse = float4(1, 0, 1, 1.9999f);

	return ReturnGeometry(p.NormalDepth.w, normal, diffuse, specular);
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
