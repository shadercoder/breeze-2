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
		float4 Tangent	: Tangent;
	)
};

struct Pixel
{
	float4 Position		: SV_Position;
	float4 NormalDepth	: TexCoord0;
	float2 TexCoord		: TexCoord1;
	
	IF_NORMALMAP(
		float4 Tangent	: TexCoord2;
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
		o.Tangent.xyz = mul(v.Tangent.xyz, (float3x3) World);
		o.Tangent.w = v.Tangent.w;
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
	float4 diffuse = DiffuseColor;
	diffuse.xyz *= FromSRGB(DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz);

	float4 specular = SpecularColor;
	IF_SPECULARMAP(
		specular.xyz *= SpecularTexture.Sample(LinearSampler, p.TexCoord).xyz;
	)

	float3 normal = normalize(p.NormalDepth.xyz);
	
	IF_NORMALMAP(
		float3 bitangent = normalize( cross(normal, p.Tangent.xyz) );
		float3 tangent = cross(bitangent, normal);

		// Texture might be flipped
		bitangent *= -sign(p.Tangent.w);

		normal = mul(
				DecodeNormal( NormalTexture.Sample(LinearSampler, p.TexCoord).xyz ),
				float3x3(tangent, bitangent, normal)
			);
		normal = normalize(normal);
	)

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
