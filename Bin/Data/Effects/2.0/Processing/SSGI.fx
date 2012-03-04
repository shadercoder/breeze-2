#define BE_SCENE_SETUP

#include <Engine/Pipe.fx>
#include <Pipelines/LPR/Scene.fx>
#include <Pipelines/LPR/Geometry.fx>

cbuffer SetupConstants
{
	float Radius
	<
		String UIName = "Radius";
	> = 0.3f;

	float Falloff
	<
		String UIName = "Falloff";
	> = 0.08f;
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
	string Format = "R16G16B16A16F";
>;
float4 AmbientTextureResolution : AmbientTargetResolution;

/// Resolved scene texture.
Texture2D ResolvedSceneTexture : ResolvedSceneTarget
<
	string Source = "SceneTarget";
	string TargetType = "Temporary";
	string Format = "R16G16B16A16F";
>;

static const int SphereKernelSize = 12;
static const float3 SphereKernel[SphereKernelSize] = {
		float3(0.083333f, 0.000000f, 0.083333f),
		float3(-0.144338f, -0.083333f, 0.166667f),
		float3(0.125000f, 0.216506f, 0.250000f),
		float3(0.000000f, -0.333333f, 0.333333f),
		float3(-0.208333f, 0.360844f, 0.416667f),
		float3(0.433013f, -0.250000f, 0.500000f),
		float3(-0.583333f, -0.000000f, 0.583333f),
		float3(0.577350f, 0.333333f, 0.666667f),
		float3(-0.375000f, -0.649519f, 0.750000f),
		float3(-0.000000f, 0.833333f, 0.833333f),
		float3(0.458333f, -0.793857f, 0.916667f),
		float3(-0.866025f, 0.500000f, 1.000000f)
	};

float4 PSAO(Pixel p) : SV_Target0
{
	// Sample random 2D matrix
	float4 noise = NoiseTexture.SampleLevel(NoiseSampler, p.Position / 32.0f, 0) * 2.0f - 1.0f;

	float4 eyeGeometry = BE_SCENE_TEXTURE(SceneGeometryTexture).SampleLevel(DefaultSampler, p.TexCoord, 0);
	float eyeDepth = ExtractDepth(eyeGeometry);
	float3 eyeNormal = ExtractNormal(eyeGeometry);

	// Project radius to scene
	float3 scaledRadius = Radius / eyeDepth;
	scaledRadius.z *= eyeDepth;

	// Normalize accumulated occlusion
	float sampleWeight = 1.0f / SphereKernelSize;

	float occlusion = 0.0f;
	float4 illumination = 0.0f;

	for(int i = 0; i < SphereKernelSize; )
	{
		float4 sampleDepth, sampleRadius;
		float4 sampleColors[4];

		// Vectorize occlusion code
		[unroll] for(int j = 0; j < 4; j++, i++)
		{
			// Randomly rotate scaled sample points
			float3 sampleOffset = scaledRadius * SphereKernel[i];
			sampleOffset.xy = sampleOffset.x * noise.xy + sampleOffset.y * noise.zw;
			
			// Sample depth texture
			sampleDepth[j] = ExtractDepth(
				BE_SCENE_TEXTURE(SceneGeometryTexture).SampleLevel(DefaultSampler, p.TexCoord + sampleOffset.xy, 0) );
			sampleRadius[j] = sampleOffset.z;

			// Sample scene texture
			sampleColors[j] = ResolvedSceneTexture.SampleLevel(DefaultSampler, p.TexCoord + sampleOffset.xy, 0);

//			float sampleDistSq = scaledRadius.z * scaledRadius.z * dot(sampleOffset.xy, sampleOffset.xy) + pow(eyeDepth - sampleDepth, 2);
//			illumination += ResolvedSceneTexture.SampleLevel(DefaultSampler, p.TexCoord + sampleOffset.xy, 0) * sampleDistSq;
		}

		// Compute occlusion
		float4 deltaDepth = eyeDepth - sampleDepth;
		float4 blocking = deltaDepth / sampleRadius;
		float4 attenuation = Falloff * deltaDepth;
		#ifdef ATAN
			float4 sampleOcclusion = atan(blocking) / (1.0f + max(0.0f, attenuation));
		#else
			float4 sampleOcclusion = blocking / ( (1.0f + abs(blocking)) * (1.0f + max(0.0f, attenuation)) );
		#endif

		occlusion += dot(sampleOcclusion, sampleWeight);

		[unroll] for(int j = 0; j < 4; j++)
			illumination += sampleOcclusion[j] * sampleColors[j];
	}

	occlusion = 1.1f * occlusion; // + 0.1f;

	return float4( saturate(illumination.xyz * sampleWeight), saturate(1 - occlusion) );
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

float4 PSGaussian(Pixel p, uniform float2 dir, uniform Texture2D sourceTex, uniform float2 sourcePixel, uniform int sourceLevel = 0) : SV_Target0
{
	float2 delta = dir * sourcePixel;

	float4 acc = 0.0f;

	[unroll] for (int i = 0; i < GaussianKernelSize; ++i)
		acc += GaussianKernel[i].y * sourceTex.SampleLevel(LinearSampler, p.TexCoord + GaussianKernel[i].x * delta, sourceLevel);
	
	return acc;
}

/// Revsub blend state.
BlendState RevSubBlendState
{
	BlendEnable[0] = true;
	SrcBlend[0] = One;
	SrcBlendAlpha[0] = Zero;
	DestBlend[0] = Src_Alpha;
	DestBlendAlpha[0] = One;
};

technique11 Default <
	bool DontFlip = true;
>
{
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 0.25f;
		float ScaleY = 0.25f;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );
		
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSAO()) );
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
		SetBlendState( RevSubBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSGaussian( float2(0.0f, 1.0f), AmbientTexture, DestinationResolution.zw )) );
	}

}
