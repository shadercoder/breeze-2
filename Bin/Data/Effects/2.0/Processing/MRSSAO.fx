#define BE_SCENE_SETUP

#include <Engine/Pipe.fx>
#include <Pipelines/LPR/Scene.fx>
#include <Pipelines/LPR/Geometry.fx>
#include "Processing/MRGeometry.fx"

cbuffer SetupConstants
{
	float4 Radius
	<
		String UIName = "Radius";
	> = float4(6.4f, 1.6f, 0.6f, 0.05f);

	float4 Falloff
	<
		String UIName = "Falloff";
	> = float4(0.5f, 1.0f, 3.5f, 7.5f);

	float4 Intensity
	<
		String UIName = "Falloff";
	> = float4(1.0f, 1.0f, 2.0f, 4.0f);

	float4 Sharpness
	<
		String UIName = "Sharpness";
	> = float4(4.0f, 10.0f, 15.0f, 25.0f);
}

Texture2D NoiseTexture
<
	string UIName = "Noise";
>;

float4 DestinationResolution : DestinationResolution;

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

SamplerState DefaultSampler
{
	Filter = MIN_MAG_MIP_POINT;
};

SamplerState NoiseSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState LinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
};

/// Intermediate target.
Texture2D AmbientTexture : AmbientTarget
<
	string TargetType = "Temporary";
	string Format = "R8G8B8A8U";
>;
float4 AmbientTextureResolution : AmbientTargetResolution;

static const int SphereKernelSize = 8;
static const float3 SphereKernel[SphereKernelSize] = {
		float3(-0.55f, -0.65f, 0.8485f), // 0.14853068170367994169333390880613
		float3(0.65f, -0.55f, 0.8485f),
		float3(-0.65f, 0.55f, 0.8485f),
		float3(0.55f, 0.65f, 0.8485f),

		float3(0.0f, -0.9f, 0.9f),
		float3(-0.9f, 0.0f, 0.9f),
		float3(0.9f, 0.0f, 0.9f),
		float3(0.0f, 0.9f, 0.9f)
	};

float4 PSAO(Pixel p, uniform Texture2D sourceTex, uniform int level = 0, uniform bool additive = false) : SV_Target0
{
	// Sample random 2D matrix
	float4 noise = NoiseTexture.SampleLevel(NoiseSampler, p.Position.xy / 32.0f, 0) * 2.0f - 1.0f;

	float4 eyeGeometry = SceneGeometryTexture.SampleLevel(DefaultSampler, p.TexCoord, 0);
	float eyeDepth = ExtractDepth(eyeGeometry);
	float3 eyeNormal = ExtractNormal(eyeGeometry);

	// Project radius to scene
	float3 scaledRadius = Radius[level] / eyeDepth;
	scaledRadius = clamp(scaledRadius, DestinationResolution.zwz, 8 * DestinationResolution.zwz);
	scaledRadius.z *= eyeDepth;

	float scaledFalloff = Falloff[level];

	// Normalize accumulated occlusion
	float sampleWeight = 1.0f / SphereKernelSize;

	float occlusion = 0.0f;

	for(int i = 0; i < SphereKernelSize; )
	{
		float4 sampleDepth, sampleRadius;

		// Vectorize occlusion code
		[unroll] for(int j = 0; j < 4; j++, i++)
		{
			// Randomly rotate scaled sample points
			float3 sampleOffset = scaledRadius * SphereKernel[i];
			sampleOffset.xy = sampleOffset.x * noise.xy + sampleOffset.y * noise.zw;
			
			// Sample depth texture
			sampleDepth[j] = ExtractDepth( sourceTex.SampleLevel(DefaultSampler, p.TexCoord + sampleOffset.xy, 0) );
			sampleRadius[j] = sampleOffset.z;
		}

		// Compute occlusion
		float4 deltaDepth = eyeDepth - sampleDepth;
		float4 blocking = deltaDepth / sampleRadius;
		float4 attenuation = scaledFalloff * deltaDepth;
		#ifdef ATAN
			float4 sampleOcclusion = atan(blocking) / (1.0f + max(0.0f, attenuation));
		#else
			float4 sampleOcclusion = blocking / ( (1.0f + abs(blocking)) * (1.0f + max(0.0f, attenuation)) );
		#endif
		sampleOcclusion *= saturate(Sharpness[level] * abs(deltaDepth));

		occlusion += dot(sampleOcclusion, sampleWeight);
	}

	occlusion = saturate( Intensity[level] * (occlusion - 0.05f) );
	occlusion = (1 - pow(1 - occlusion, 2));

	return (additive)
		? saturate( occlusion + AmbientTexture.SampleLevel(LinearSampler, p.TexCoord, 0) )
		: occlusion;
}

static const int GaussianKernelSize = 7;
static float2 GaussianKernel[GaussianKernelSize] = {
		float2(-3.0f, 0.015625f),
		float2(-2.0f, 0.09375f),
		float2(-1.0f, 0.234375f),
		float2(0.0f, 0.3125f),
		float2(1.0f, 0.234375f),
		float2(2.0f, 0.09375f),
		float2(3.0f, 0.015625f)
	};

float4 PSGaussian(Pixel p, uniform float2 dir, uniform Texture2D sourceTex, uniform float2 sourcePixel, uniform bool inv = false, uniform int sourceLevel = 0) : SV_Target0
{
	float2 delta = dir * sourcePixel;

	float4 acc = 0.0f;

	[unroll] for (int i = 0; i < GaussianKernelSize; ++i)
		acc += GaussianKernel[i].y * sourceTex.SampleLevel(LinearSampler, p.TexCoord + GaussianKernel[i].x * delta, sourceLevel);
	
	return (inv) ? 1 - acc : acc;
}

float4 PSDepth(Pixel p, uniform Texture2D sourceTex, uniform float scale, uniform int sourceLevel = 0) : SV_Target0
{
	return ExtractDepth( sourceTex.SampleLevel(DefaultSampler, p.TexCoord, sourceLevel) ) * scale;
}

/// Revsub blend state.
BlendState RevMultBlendState
{
	BlendEnable[0] = true;
	SrcBlend[0] = Zero;
	SrcBlendAlpha[0] = Zero;
	DestBlend[0] = Inv_Src_Alpha;
	DestBlendAlpha[0] = One;
};

technique11 Default <
	bool DontFlip = true;
>
{
/*	pass <
		string Color0 = "SceneTarget";
		bool bKeepColor0 = true;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );
		
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSDepth(SceneGeometryTexture16, 0.02f)) );
	}
*/

	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f / 16;
		float ScaleY = 1.0f / 16;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );
		
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSAO(SceneGeometryTexture16, 0)) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(1.0f, 0.0f), AmbientTexture, DestinationResolution.zw )) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(0.0f, 1.0f), AmbientTexture, DestinationResolution.zw )) );
	}
	
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 2.0f;
		float ScaleY = 2.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSAO(SceneGeometryTexture8, 1, true)) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(1.0f, 0.0f), AmbientTexture, DestinationResolution.zw )) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(0.0f, 1.0f), AmbientTexture, DestinationResolution.zw )) );
	}

	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 2.0f;
		float ScaleY = 2.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSAO(SceneGeometryTexture4, 2, true)) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(1.0f, 0.0f), AmbientTexture, DestinationResolution.zw )) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(0.0f, 1.0f), AmbientTexture, DestinationResolution.zw )) );
	}

	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 2.0f;
		float ScaleY = 2.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSAO(SceneGeometryTexture2, 3, true)) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(1.0f, 0.0f), AmbientTexture, DestinationResolution.zw )) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(0.0f, 1.0f), AmbientTexture, DestinationResolution.zw )) );
	}

	pass <
		string Color0 = "AmbientTarget";
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(1.0f, 0.0f), AmbientTexture, DestinationResolution.zw )) );
	}

	pass <
		string Color0 = "SceneTarget";
		bool bKeepColor0 = true;
	>
	{
//		SetBlendState( RevMultBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(0.0f, 1.0f), AmbientTexture, DestinationResolution.zw, true )) );
	}

}
