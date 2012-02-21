#define BE_SCENE_SETUP

#include <Engine/Pipe.fx>
#include <Pipelines/Scene.fx>

struct Vertex
{
	float4 Position : Position;
};

struct Pixel
{
	float4 Position : SV_Position;
	float2 TexCoord : TexCoord0;
};

Pixel VSQuad(Vertex v)
{
	Pixel p;

	p.Position = v.Position;
	p.TexCoord = 0.5f + float2(0.5f, -0.5f) * v.Position.xy;

	return p;
}

SamplerState LinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
};

float4 PSTonemap(Pixel p) : SV_Target0
{
	float4 hdrColor = SceneTexture.Sample(LinearSampler, p.TexCoord);
	float hdrLum = dot( hdrColor, float3(0.2126f, 0.7152f, 0.0722f) );
//	hdrColor.xyz = hdrColor.xyz / (1.0f + hdrLum);
	hdrColor.xyz = 1 - exp2(-hdrColor.xyz);
	return hdrColor;
}

technique11 Default <
	bool DontFlip = true;
>
{
	pass <
		string Color0 = "FinalTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSTonemap()) );
	}
}
