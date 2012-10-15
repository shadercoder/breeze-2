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
	> = float4(1.0f, 1.0f, 1.0f, 0.05f);

	float4 SpecularColor
	<
		String UIName = "Specular";
		String UIWidget = "Color";
	> = float4(0.0f, 0.0f, 0.0f, 0.1f);
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

	float3 diffuseTex = DiffuseTexture.Sample(LinearSampler, p.TexCoord).xyz;

	// Grass is darker when looked at top-down
	float3 angleDiffuseTex = pow(diffuseTex, 2) * 0.9f;
	float angleBlend = saturate( dot(normal, -normalize(p.World.xyz - Perspective.CamPos.xyz)) );
	diffuseTex = lerp( diffuseTex, angleDiffuseTex, oneMinusPow(angleBlend, 2) );

	diffuse.xyz *= diffuseTex;

	return ReturnGeometry(p.NormalDepth.w, normal, diffuse, SpecularColor);
}

struct GrassVertex
{
	float4 Position	: Position;
	float3 Normal	: Normal;
};

struct GrassControlPoint
{
	float4 Position	: Position;
	float3 Normal	: Normal;
};

GrassControlPoint VSGrass(GrassVertex v)
{
	GrassControlPoint o;
	o.Position = mul(v.Position, World);
	o.Normal = mul((float3x3) WorldInverse, v.Normal);
	return o;
}

struct GrassConstants
{
	float Edges[3]	: SV_TessFactor;
	float Inside	: SV_InsideTessFactor;
};

int mod3(int i) { return i % 3; }

GrassConstants HSGrassConstants(InputPatch<GrassControlPoint, 3> p, uint PatchID : SV_PrimitiveID)
{
	GrassConstants o;

	o.Inside = 0.0f;

	for (int i = 0; i < 3; ++i)
	{
		o.Edges[i] = 32 * length(p[mod3(i + 2)].Position.xyz - p[mod3(i + 1)].Position.xyz);
		o.Inside += o.Edges[i];
	}

	o.Inside /= 3;

	return o;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSGrassConstants")]
GrassControlPoint HSGrass(InputPatch<GrassControlPoint, 3> inputPatch, uint inputIdx : SV_OutputControlPointID)
{
	return inputPatch[inputIdx];
}

[domain("tri")]
GrassControlPoint DSGrass(GrassConstants hull, OutputPatch<GrassControlPoint, 3> inputPatch, float3 location : SV_DomainLocation)
{
	GrassControlPoint o;

	o.Position = location.x * inputPatch[0].Position
		+ location.y * inputPatch[1].Position
		+ location.z * inputPatch[2].Position;
	o.Normal = inputPatch[0].Normal;

	return o;
}

float3 rand(float3 x)
{
	x = float3(
			dot(x, 521),
			dot(x, 571),
			dot(x, 547)
		);

	return frac( abs( x ) );
}

[MaxVertexCount(4)]
void GSGrass(triangle GrassControlPoint tri[3], inout TriangleStream<Pixel> triStream)
{
	float3 inputNormal = normalize(tri[0].Normal);

	float4 position = tri[0].Position;
	float3 offset = 0.05f * rand(position.xyz);
	offset -= inputNormal * dot(offset, inputNormal);
	position.xyz += offset;

	float3 tangent = inputNormal;
	tangent += rand(position.xyz) * 0.2f - 0.1f;
	tangent = normalize(tangent);

	float3 normal = rand(position.xyz + tangent) * 2.0f - 1.0f;
	if (dot(normal, position.xyz - Perspective.CamPos.xyz) > 0.0f)
		normal = -normal;

	float3 bitangent = normalize( cross(normal, tangent) );
	normal = cross(tangent, bitangent);
	
	float4 tanoffset = float4(tangent * 0.4f, 0);
	float4 bioffset = float4(bitangent * 0.02f, 0);

	float4 p[4];
	p[0] = position - bioffset;
	p[1] = position - bioffset * 0.1f + tanoffset;
	p[2] = position + bioffset;
	p[3] = position + bioffset * 0.1f + tanoffset;

	for (int i = 0; i < 4; ++i)
	{
		Pixel o;
		o.World = position;
		o.Position = mul(p[i], Perspective.ViewProj);
		o.NormalDepth.x = o.Position.w;
		o.NormalDepth.yzw = normal;
		o.TexCoord = 0.0f;

		triStream.Append(o);
	}
}

GeometryOutput PSGrassGeometry(Pixel p)
{
	float3 normal = normalize(p.NormalDepth.xyz);
	float4 diffuse = DiffuseColor;

	float3 diffuseTex = DiffuseTexture.Sample(LinearSampler, p.World.xz).rgb;
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
		SetPixelShader( CompileShader(ps_4_0, PSGeometry()) );
	}
}

technique11 Grass <
	string PipelineStage = "GeometryPipelineStage";
	string RenderQueue = "DefaultRenderQueue";
>
{
	pass
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetPixelShader( CompileShader(ps_4_0, PSGeometry()) );
	}

/*	pass < int HSControlPoints = 3; >
	{
		SetVertexShader( CompileShader(vs_4_0, VSGrass()) );
		SetHullShader( CompileShader(hs_5_0, HSGrass()) );
		SetDomainShader( CompileShader(ds_5_0, DSGrass()) );
		SetGeometryShader( CompileShader(gs_5_0, GSGrass()) );
		SetPixelShader( CompileShader(ps_4_0, PSGrassGeometry()) );
	}
*/
}

technique11 <
	string IncludeEffect = "Prototypes/Shadow.fx";
> { }

technique11 <
	string IncludeEffect = "Prototypes/Feedback.fx";
> { }
