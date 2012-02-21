#define BE_RENDERABLE_INCLUDE_WORLD
#define BE_RENDERABLE_INCLUDE_PROJ
#define BE_RENDERABLE_INCLUDE_ID

#include <Engine/Perspective.fx>
#include <Engine/Renderable.fx>

cbuffer SetupConstants
{
	float4 Color
	<
		String UIName = "Color";
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
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = mul(v.Position, WorldViewProj);
	o.Position -= WorldViewProj[3];
	o.Position *= WorldViewProj[3][3] * 0.15f;
	o.Position += WorldViewProj[3];
	o.Position.z = 0.0f;
	
	return o;
}

float4 PSMain(Pixel p) : SV_Target0
{
	return Color;
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
	string PipelineStage = "DefaultPipelineStage";
	string RenderQueue = "AlphaRenderQueue";
>
{
	pass
	{
		SetBlendState( AlphaBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
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
