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

float4 DestinationResolution : DestinationResolution;

SamplerState LinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
};

float4 PSInterleave(Pixel p, uniform float width, uniform float dir) : SV_Target0
{
	float destFrac = pow(2.0f, -width);

	float2 texPos = p.TexCoord * DestinationResolution;
	
	texPos += step( frac(destFrac * texPos), 0.4999f )
		* (dir * destFrac) * DestinationResolution;

	[unroll] if (dir < 0.0f)
		texPos += (texPos < 0) * DestinationResolution;
	[unroll] if (dir > 0.0f)
		texPos -= (texPos >= DestinationResolution) * DestinationResolution;

	return SceneTexture[(uint2) texPos];
}

float4 PSTest(Pixel p, uniform float frac) : SV_Target0
{
	float4 color = SceneTexture.SampleLevel(LinearSampler, p.TexCoord, 0);

	if (p.TexCoord.x < frac && p.TexCoord.y < frac)
		color *= 2.0f;

	return color;
}

static const int ScramblingPermutationLength = 32;
static const int ScramblingPermutation[ScramblingPermutationLength] = {
	16, 13, 10, 17, 23, 15, 27, 30, 18,
	19, 2, 4, 26, 3, 12, 14, 29, 24, 6, 7, 9, 11, 5,
	8, 21, 28, 25, 31, 1, 20, 22, 0
 };
static const int ScramblingPermutationInv[ScramblingPermutationLength] = {
	31, 28, 10, 13, 11, 22, 18, 19, 23, 20, 2,
	21, 14, 1, 15, 5, 0, 3, 8, 9, 29, 24, 30,
	4, 17, 26, 12, 6, 25, 16, 7, 27
 };

float4 PSTile(Pixel p, uniform int permutation[ScramblingPermutationLength]) : SV_Target0
{
	float2 tilePos = p.TexCoord * ScramblingPermutationLength;
	
	int2 tileIdx = clamp( (int2) tilePos, 0, ScramblingPermutationLength - 1 );

	int2 tileIdxR = tileIdx;
//	tileIdxR += tileIdx.yx * int2(41, 41);
	tileIdxR %= ScramblingPermutationLength;

	int2 newTileIdx = int2( permutation[tileIdxR.x], permutation[tileIdxR.y] );

	float2 newTilePos = tilePos + (newTileIdx - tileIdx);

	float2 newCoord = newTilePos / ScramblingPermutationLength;

	return SceneTexture.Sample(LinearSampler, newCoord);
}

float4 PSScramble(Pixel p, uniform int permutation[ScramblingPermutationLength]) : SV_Target0
{
	float2 pixPos = p.TexCoord * DestinationResolution;
	
	int2 pixPosI = (int2) clamp(pixPos, 0, DestinationResolution - 1);
	int2 pixIdx = pixPosI % ScramblingPermutationLength;
	
	int2 pixIdxR = pixIdx;
//	pixIdxR += pixPosI.yx * int2(41, 41);
	pixIdxR %= ScramblingPermutationLength;
	
	int2 newPixIdx = int2( permutation[pixIdxR.x], permutation[pixIdxR.y] );

//	float2 test = (newPixIdx - pixIdx) * (0.5f / ScramblingPermutationLength) + 0.5f;
//	return test.xyxy;

	float2 newPixPos = pixPos + (newPixIdx - pixIdx);

	float2 newCoord = newPixPos / DestinationResolution;

	return SceneTexture.Sample(LinearSampler, newCoord);
}

technique11 Default <
	bool DontFlip = true;
>
{
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSInterleave(1.0f, 1.0f)) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSInterleave(2.0f, 1.0f)) );
	}

	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSInterleave(3.0f, 1.0f)) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSTest(1.0f/8.0f)) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSInterleave(3.0f, -1.0f)) );
	}

	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSInterleave(2.0f, -1.0f)) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSInterleave(1.0f, -1.0f)) );
	}

/*
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSTile(ScramblingPermutation)) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSScramble(ScramblingPermutation)) );
	}
*/	/*
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSScramble(ScramblingPermutationInv)) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bClearColor0 = true;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSTile(ScramblingPermutationInv)) );
	}*/
}
