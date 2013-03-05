#define BE_RENDERABLE_INCLUDE_WORLD
#define BE_RENDERABLE_INCLUDE_VIEW
#define BE_RENDERABLE_INCLUDE_PROJ
#define BE_RENDERABLE_INCLUDE_ID

#include <Engine/Perspective.fx>
#include <Engine/Renderable.fx>

cbuffer SetupConstants
{
	float4 Color
	<
		String UIName = "Color";
		String UIWidget = "Color";
	> = float4(1.0f, 0.0f, 0.0f, 0.7f);
}

struct Vertex
{
	float4 Position	: Position;
	float3 Normal	: Normal;
};

struct Pixel
{
	float4 Position		: SV_Position;
	float3 CamNormal	: TexCoord0;
#ifdef DARKEN_SPHERE_BACK
	float3 CamPos		: TexCoord1;
#endif
};

Pixel VSMain(Vertex v, uniform bool bKeepDepth = false)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.Position -= WorldViewProj[3];
	o.Position *= (sqrt(0.9f + WorldViewProj[3][3]) - 0.9f) * 1.1f;
	o.Position += WorldViewProj[3];
	if (!bKeepDepth)
		o.Position.z = 0.0f;

	o.CamNormal = mul(mul((float3x3) WorldInverse, v.Normal), (float3x3) Perspective.View);
#ifdef DARKEN_SPHERE_BACK
	o.CamPos = mul(v.Position.xyz, (float3x3) WorldView);
#endif
	
	return o;
}

float4 PSMain(Pixel p) : SV_Target0
{
	float3 camNormal = normalize(p.CamNormal);

	float4 color = Color;
#ifdef DARKEN_SPHERE_BACK
	if (p.CamPos.z > 0.0f)
	{
		color.xyz *= 0.01f;
		color.a *= step( 0.95f * saturate( 1.0f - dot(normalize(p.CamPos), -camNormal) ), -camNormal.z );
	}
#endif
	color.xyz *= saturate(-camNormal.z) * color.a;
	return color;
}

uint4 PSObjectIDs(float4 p : SV_Position) : SV_Target0
{
	return ObjectID;
}

/// Alpha blend state.
BlendState AlphaBlendState
{
	BlendEnable[0] = true;
	SrcBlend[0] = One;
	DestBlend[0] = Inv_Src_Alpha;
};

technique11 Overlay <
	string PipelineStage = "OverlayPipelineStage";
	string RenderQueue = "AlphaRenderQueue";
>
{
	pass
	{
		SetBlendState( AlphaBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSMain(true)) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSMain()) );
	}
}

technique11 ObjectIDs <
	string PipelineStage = "ObjectIDPipelineStage";
	string RenderQueue = "AlphaRenderQueue";
>
{
	pass
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSObjectIDs()) );
	}
}
