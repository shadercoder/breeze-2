#define BE_SCENE_SETUP
#define BE_PERSPECTIVE_SETUP

#include <Engine/Pipe.fx>
#include <Engine/Perspective.fx>
#include <Pipelines/LPR/Scene.fx>
#include <Pipelines/LPR/Geometry.fx>

#include "Processing/BilateralAverage.fx"

cbuffer SetupConstants
{
	float Intensity
	<
		String UIName = "Intensity";
	> = 1.5f;

	float MaxOcclusion
	<
		String UIName = "Max Occlusion";
	> = 0.85f;

	float Radius
	<
		String UIName = "Radius";
	> = 1.5f;
	
	float DetailRadius
	<
		String UIName = "Detail Radius";
	> = 0.3f;

	float Falloff
	<
		String UIName = "Falloff";
	> = 7.5f; // 44.5f; // 7.5f;

	float DetailFalloff
	<
		String UIName = "Detail Falloff";
	> = 15; // 1100.0f; // 15

	float CloseAttenuation
	<
		String UIName = "Close Attenuation";
	> = 0.08f;

	float Sharpness
	<
		String UIName = "Sharpness";
	> = 50.0f;

	float DetailSharpness
	<
		String UIName = "Detail Sharpness";
	> = 250.0f;
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
	float3 CamDir : TexCoord1;
};

Pixel VSQuad(Vertex v)
{
	Pixel o;

	o.Position = v.Position;
	o.TexCoord = 0.5f + float2(0.5f, -0.5f) * v.Position.xy;

	o.CamDir = v.Position.xyw * float3(Perspective.ProjInv[0][0], Perspective.ProjInv[1][1], 1.0f);
	o.CamDir = mul(o.CamDir, (float3x3) Perspective.ViewInv);

	return o;
}

SamplerState DefaultSampler;

SamplerState NoiseSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

/// Intermediate target.
Texture2D AmbientTexture : AmbientTarget
<
	string TargetType = "Permanent";
	string Format = "R8G8B8A8U";
>;
float4 AmbientTextureResolution : AmbientTargetResolution;

static const int PoissonKernelSize = 8;
static const float2 PoissonKernel[PoissonKernelSize] = {
  float2(-0.07280093f, 0.38396510f),
  float2(-0.88017300f, 0.39960410f),
  float2(-0.15059570f,-0.69103160f),
  float2( 0.36638260f,-0.58297690f),
  float2( 0.80362130f, 0.18440510f),
  float2( 0.32283370f, 0.35518560f),
  float2(-0.70078040f,-0.12033080f),
  float2( 0.14763900f, 0.74279390f),
};
/*
static const int SphereKernelSize = 12;
static const float3 SphereKernel[SphereKernelSize] = {
		float3(0.083333f, 0.000000f, 0.083333f),
		0.1f * float3(-0.144338f, -0.083333f, 0.166667f),
		0.1f * float3(0.125000f, 0.216506f, 0.250000f),
		float3(0.000000f, -0.333333f, 0.333333f),
		float3(-0.208333f, 0.360844f, 0.416667f),
		0.1f * float3(0.433013f, -0.250000f, 0.500000f),
		float3(-0.583333f, -0.000000f, 0.583333f),
		float3(0.577350f, 0.333333f, 0.666667f),
		0.1f * float3(-0.375000f, -0.649519f, 0.750000f),
		float3(-0.000000f, 0.833333f, 0.833333f),
		float3(0.458333f, -0.793857f, 0.916667f),
		float3(-0.866025f, 0.500000f, 1.000000f)
	};
*/

float4 PSAO(Pixel p, uniform float2 centerOffset = 0.0f) : SV_Target0
{
	// Sample random 2D matrix
	float4 noise = NoiseTexture.SampleLevel(NoiseSampler, p.Position.xy / 32.0f, 0) * 2.0f - 1.0f;

	float4 eyeGeometry = BE_SCENE_TEXTURE(SceneGeometryTexture).SampleLevel(DefaultSampler, p.TexCoord + centerOffset, 0);
	float eyeDepth = ExtractDepth(eyeGeometry);
	float3 eyeNormal = ExtractNormal(eyeGeometry);
	
	float closeness = saturate(CloseAttenuation * eyeDepth);
	float closenessSq = pow(closeness, 2);

	float3 eyePos = p.CamDir * eyeDepth;
	float3 camDirDDU = ddx(p.CamDir) / ddx(p.TexCoord.x);
	float3 camDirDDV = ddy(p.CamDir) / ddy(p.TexCoord.y);

	float3 minRadius = 3 * DestinationResolution.zwz;
	float3 maxRadius = 256 * DestinationResolution.zwz;

	// Project radius to scene
	float3 scaledRadius = lerp(DetailRadius, Radius, closenessSq) / eyeDepth;
//	scaledRadius = minRadius + saturate( pow( (scaledRadius - minRadius) / (maxRadius - minRadius), 2 ) ) * (maxRadius - minRadius);
	scaledRadius = clamp(scaledRadius, minRadius, maxRadius);
	scaledRadius.z *= eyeDepth;

	float scaledBias = -1.0e-8f * eyeDepth;

	float scaledFalloff = lerp(DetailFalloff, Falloff, closenessSq);
	float scaledSharpness = lerp(DetailSharpness, Sharpness, closeness);

	// Normalize accumulated occlusion
	float sampleWeight = 1.0f / PoissonKernelSize;

	float occlusion = 0.0f;
	float clipping = 0.0f;

	[unroll] for(int i = 0; i < PoissonKernelSize; ++i)
	{
		// Randomly rotate scaled sample points
		float3 sampleOffset = scaledRadius * PoissonKernel[i].xyx;
		sampleOffset.xy = sampleOffset.x * noise.xy + sampleOffset.y * noise.zw;

		float2 sampleCoord = p.TexCoord + sampleOffset.xy;

		// Clip, if samples too far out
		float4 sampleClip = max(0.0f, float4(sampleCoord - 0.5f, 0.5f - sampleCoord));
		clipping += dot(sampleClip, 1.0f);

		float sampleDepth = ExtractDepth(
			BE_SCENE_TEXTURE(SceneGeometryTexture).SampleLevel(DefaultSampler, sampleCoord, 0) );
		
		// Compute sample position
		float3 sampleCamDir = p.CamDir + sampleOffset.x * camDirDDU + sampleOffset.y * camDirDDV;
		float3 samplePos = sampleCamDir * sampleDepth;
		
		float3 toSample = samplePos - eyePos;
		float3 sampleDir = normalize(toSample);

		float cosAngle = dot(sampleDir, eyeNormal);
		float sampleDistSq = dot(toSample, toSample);
		float attenuation = scaledFalloff * sampleDistSq;
		float sampleOcclusion = max( cosAngle + scaledBias, 0.0f ) / (1.0f + sampleDistSq);

//		sampleOcclusion *= saturate(scaledSharpness * abs(sampleDistSq));

		occlusion += sampleOcclusion;
	}

	clipping *= sampleWeight;
	occlusion *= saturate(1 - clipping);

//	float scaledIntensity = lerp(DetailIntensity, Intensity, closeness);

//	occlusion = saturate(occlusion - 0.01f);
	occlusion *= 2 * sampleWeight;
	occlusion = saturate(Intensity * occlusion);
	occlusion = 1 - pow(1 - occlusion, 3);
	occlusion = min(occlusion, MaxOcclusion);

//	occlusion = saturate( 1.5f * occlusion );
//	float occlusionClose = saturate( 1.5f * sqrt(occlusion) );
//	float occlusionDist = 1 - pow(1 - occlusion, 4);
//	occlusion = lerp( occlusionClose, occlusionDist, closeness );

	return 1 - occlusion;
}


float4 PSBlit(float4 Position : SV_Position, float2 TexCoord : TexCoord0,
	uniform Texture2D sourceTex, uniform SamplerState sourceSampler, uniform int sourceLevel = 0) : SV_Target0
{
	return sourceTex.SampleLevel(sourceSampler, TexCoord, sourceLevel);
}

float4 PSBilateralUpsample(float4 Position : SV_Position, float2 TexCoord : TexCoord0,
	uniform Texture2D geometryTex, uniform SamplerState geometrySampler, uniform float2 geometryOffset, 
	uniform Texture2D sourceTex, uniform SamplerState sourceSampler, uniform float2 sourcePixel, uniform int sourceLevel = 0) : SV_Target0
{
	float2 sourceGeometryTexCoord = (floor(TexCoord / sourcePixel) + 0.5f) * sourcePixel + geometryOffset;

	float4 eyeGeometry = geometryTex.SampleLevel(geometrySampler, TexCoord, 0);
	float eyeDepth = ExtractDepth(eyeGeometry);
	float3 eyeNormal = ExtractNormal(eyeGeometry);

	float4 sourceGeometry = geometryTex.SampleLevel(geometrySampler, sourceGeometryTexCoord, 0);
	float sourceDepth = ExtractDepth(sourceGeometry);
	float3 sourceNormal = ExtractNormal(sourceGeometry);

	float sourceMatch = saturate( 1 - abs(sourceDepth - eyeDepth) )
			* saturate( dot(eyeNormal, sourceNormal) );

	float2 maxSampleMatchOffset = 0.0f;
	float maxSampleMatch = sourceMatch;

//	if (sourceMatch < 0.5f)
	{
		float2 sampleDeltas[] = { float2(0.0f, -1.0f), float2(-1.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f) };

		[unroll] for (int i = 0; i < 4; ++i)
		{
			float2 sampleOffset = sourcePixel * sampleDeltas[i];

			float4 sampleGeometry = geometryTex.SampleLevel(geometrySampler, sourceGeometryTexCoord + sampleOffset, 0);
			float sampleDepth = ExtractDepth(sampleGeometry);
			float3 sampleNormal = ExtractNormal(sampleGeometry);

			float sampleMatch = saturate( 1 - abs(sampleDepth - eyeDepth) )
					* saturate( dot(eyeNormal, sampleNormal) );

			maxSampleMatch = max(sampleMatch, maxSampleMatch);
			maxSampleMatchOffset = (sampleMatch == maxSampleMatch) ? sampleOffset : maxSampleMatchOffset;
		}
	}

	return sourceTex.SampleLevel(sourceSampler, TexCoord + maxSampleMatchOffset, sourceLevel);
}

/// Revsub blend state.
BlendState RevSubBlendState
{
	BlendEnable[0] = true;
	SrcBlend[0] = Zero;
	SrcBlendAlpha[0] = Zero;
	DestBlend[0] = Src_Alpha;
	DestBlendAlpha[0] = One;
};

technique11 Default <
	bool DontFlip = true;
	string PipelineStage = "GeometryPipelineStage";
//	string PipelineStage = "DefaultPipelineStage";
>
{
	pass <
/*		string Color0 = "SceneTarget";
		bool bKeepColor0 = true;
*/
		string Color0 = "AmbientTarget";
		float ScaleX = 0.5f;
		float ScaleY = 0.5f;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );
		
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSAO(DestinationResolution.zw / 4)) );
	}
	
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0,
				PSBilateralAverage(
					float2(1.0f, 0.0f), 4, 3,
					BE_SCENE_TEXTURE(SceneGeometryTexture), DefaultSampler,
					AmbientTexture, DefaultSampler, DestinationResolution.zw,
					AmbientTextureResolution.zw / 4
				)
			) );
	}
	pass <
		string Color0 = "AmbientTarget";
		float ScaleX = 1.0f;
		float ScaleY = 1.0f;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0,
				PSBilateralAverage(
					float2(0.0f, 1.0f), 4, 3,
					BE_SCENE_TEXTURE(SceneGeometryTexture), DefaultSampler,
					AmbientTexture, DefaultSampler, DestinationResolution.zw,
					AmbientTextureResolution.zw / 4
				)
			) );
	}
	pass <
/*		string Color0 = "SceneTarget";
		bool bKeepColor0 = true;
*/		
		string Color0 = "AmbientTarget";
		float ScaleX = 2.0f;
		float ScaleY = 2.0f;
	>
	{
//		SetBlendState( RevSubBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0,
				PSBilateralUpsample(
					BE_SCENE_TEXTURE(SceneGeometryTexture), DefaultSampler, AmbientTextureResolution.zw / 4,
					AmbientTexture, DefaultSampler, AmbientTextureResolution.zw
				)
			) );
	}

/*	pass <
		string Color0 = "SceneTarget";
		bool bKeepColor0 = true;
	>
	{
//		SetBlendState( RevSubBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0,
				PSBlit(
					AmbientTexture, DefaultSampler
				)
			) );
	}
*/
}
