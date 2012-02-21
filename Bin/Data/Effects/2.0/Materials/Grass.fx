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
	> = float4(1.0f, 1.0f, 1.0f, 0.05f);

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
	float4 World		: TexCoord0;
	float4 NormalDepth	: TexCoord1;
	float2 TexCoord		: TexCoord2;
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.World = mul(v.Position, World);
	o.NormalDepth.xyz = mul((float3x3) WorldInverse, v.Normal);
	o.NormalDepth.w = o.Position.w;
	o.TexCoord = o.World.xz / 4; // v.TexCoord;
	
	return o;
}

SamplerState LinearSampler
{
	Filter = ANISOTROPIC;
	AddressU = WRAP;
	AddressV = WRAP;
};

float oneMinusPow(float val, float powVal)
{
	return 1.0f - pow(1.0f - val, powVal);
}

GeometryOutput PSGeometry(Pixel p)
{
	float3 normal = normalize(p.NormalDepth.xyz);
	float4 diffuse = DiffuseColor;

	float3 diffuseTex = FromSRGB(DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz);

	// Grass is darker when looked at top-down
	float3 angleDiffuseTex = pow(diffuseTex, 2) * 0.9f;
	float angleBlend = saturate( dot(normal, -normalize(p.World.xyz - Perspective.CamPos.xyz)) );
	diffuseTex = lerp( diffuseTex, angleDiffuseTex, oneMinusPow(angleBlend, 2) );

	diffuse.xyz *= diffuseTex;

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
