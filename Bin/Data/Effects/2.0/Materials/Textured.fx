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
	> = float4(0.0f, 0.0f, 0.0f, 0.1f);

	float2 TextureRepeat
	<
		String UIName = "Texture Repeat";
	> = float2(1.0f, 1.0f);
}

#ifndef NOCOLORMAP

	Texture2D DiffuseTexture
	<
		string UIName = "Diffuse";
	>;

	#define IF_COLORMAP(x) x
	#define IF_NOCOLORMAP(x)

#else

	#define IF_COLORMAP(x)
	#define IF_NOCOLORMAP(x) x

#endif

#ifdef NORMALMAP

	float4 NormalBlend
	<
		String UIName = "Normal Blend";
	> = float4(0.0f, 0.0f, 0.0f, 1.0f);

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
		o.Tangent.xyz = mul(v.Tangent, (float3x3) World);
		// Extract handedness
		o.Tangent.w = sign(dot(v.Tangent, v.Tangent) - 0.5f);
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
	
	IF_COLORMAP(
		diffuse.xyz *= FromSRGB(DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz);
	)

	float4 specular = SpecularColor;
	IF_SPECULARMAP(
		specular.xyz *= SpecularTexture.Sample(LinearSampler, p.TexCoord).xyz;
	)

	float3 normal = normalize(p.NormalDepth.xyz);
	
	IF_NORMALMAP(
		float3 bitangent = normalize( cross(normal, p.Tangent.xyz) );
		float3 tangent = cross(bitangent, normal);

		// Texture might be flipped
		// ORDER: Don't flip before full frame has been computed!
		bitangent *= p.Tangent.w;
		
		float4 normalSample = NormalTexture.Sample(LinearSampler, p.TexCoord);
		
		float3 mapNormal = mul(
				DecodeNormal( normalSample.xyz ),
				float3x3(tangent, bitangent, normal)
			);
		normal = normalize( lerp(normal, mapNormal, NormalBlend.w) );
		
		IF_NOCOLORMAP(
			float3 normalBlend = smoothstep(0.1f * NormalBlend.xyz, NormalBlend.xyz, normalSample.w);
			diffuse.xyz *= normalBlend;
			specular.xyz *= normalBlend;
		)
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
