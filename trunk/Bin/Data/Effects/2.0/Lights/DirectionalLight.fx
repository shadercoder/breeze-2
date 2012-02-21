#include <Engine/Perspective.fx>
#include <Engine/DirectionalLight.fx>
#include <Pipelines/LPR/Scene.fx>
#include <Pipelines/LPR/Geometry.fx>

#include "Processing/BilateralAverage.fx"

cbuffer Light0
{
	DirectionalLightLayout DirectionalLight;
}

Texture2DArray DirectionalLightShadowMaps : ShadowMaps0;

// TODO: Check if available?
Texture2D AmbientTexture : AmbientTarget
<
	string TargetType = "Permanent";
	string Format = "R8G8B8A8U";
>;

Texture2D NoiseTexture
<
	string UIName = "Noise";
>;

Texture2D ShadowTexture : ShadowTarget
<
	string TargetType = "Temporary";
	string Format = "R8G8B8A8U";
>;

float4 DestinationResolution : DestinationResolution;

struct Vertex
{
	float4 Position	: Position;
};

struct Pixel
{
	float4 Position		: SV_Position;
	float2 TexCoord		: TexCoord0;
	float3 CamDir		: TexCoord1;
};

Pixel VSMain(Vertex v)
{
	Pixel o;
	
	o.Position = v.Position;
	o.Position.z = 0.0f;
	o.TexCoord = 0.5f + float2(0.5f, -0.5f) * v.Position.xy;
	
	o.CamDir = v.Position.xyw * float3(Perspective.ProjInv[0][0], Perspective.ProjInv[1][1], 1.0f);
	o.CamDir = mul(o.CamDir, (float3x3) Perspective.ViewInv);

	return o;
}

SamplerState DefaultSampler;

SamplerState NoiseSampler
{
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState ShadowSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
};

static const int PoissonKernelSize = 8;

static const float2 PoissonKernel[PoissonKernelSize] = {
  float2(-0.07280093f, 0.38396510f),
  float2(-0.88017300f, 0.39960410f),
  float2(-0.15059570f,-0.69103160f),
  float2( 0.36638260f,-0.58297690f),
  float2( 0.80362130f, 0.18440510f),
  float2( 0.32283370f, 0.35518560f),
  float2(-0.70078040f,-0.12033080f),
  float2( 0.14763900f, 0.74279390f)
};
/*
static const float2 PoissonKernel[PoissonKernelSize] = {
  0.125f * float2( 1.0f, 0.0f),
  1.0f * float2( 0.7071f, 0.7071f ),
  0.25f * float2( 0.0f, 1.0f ),
  0.875f * float2( -0.7071f, 0.7071f ),
  0.375f * float2( -1.0f, 0.0f), 0.625f * 
  0.5f * float2( -0.7071f, -0.7071f ),
  0.625f * float2( 0.0f, -1.0f ),
  0.75f * float2( 0.7071f, -0.7071f ),
};
*/
#ifdef NONONO
static const float2 PoissonKernel[PoissonKernelSize] = {
  /* 0.15f */ float2( 1.0f, 0.0f),
  /* 1.0f */ float2( 0.7071f, 0.7071f ),
  /* 0.3f */ float2( 0.0f, 1.0f ),
  /* 0.9f */ float2( -0.7071f, 0.7071f ),
  /* 0.5f */ float2( -1.0f, 0.0f), 0.625f * 
  /* 0.8f */ float2( -0.7071f, -0.7071f ),
  /* 0.7f */ float2( 0.0f, -1.0f ),
  /* 0.7f */ float2( 0.7071f, -0.7071f ),
};
#endif

static const float2 SignedPoissonKernel[PoissonKernelSize] = {
  float2(-0.07280093f, 0.38396510f),
  float2(-0.88017300f, 0.39960410f),
  float2(-0.15059570f, 0.69103160f),
  float2( 0.36638260f, 0.58297690f),
  float2( 0.80362130f, 0.18440510f),
  float2( 0.32283370f, 0.35518560f),
  float2(-0.70078040f, 0.12033080f),
  float2( 0.14763900f, 0.74279390f)
};

float4 PSShadow(Pixel p) : SV_Target0
{
	float4 eyeGeometry = BE_SCENE_TEXTURE(SceneGeometryTexture).SampleLevel(DefaultSampler, p.TexCoord, 0);
	float eyeDepth = ExtractDepth(eyeGeometry);
	
	int splitIndex = (int) dot(eyeDepth >= DirectionalLight.ShadowSplitPlanes, 1.0f);

	clip(3 - splitIndex);

	float3 world = Perspective.CamPos.xyz + p.CamDir * eyeDepth;
	float3 worldNormal = ExtractNormal(eyeGeometry);

	float4 shadowCoord = mul(float4(world + 0.1f * worldNormal, 1.0f), DirectionalLight.ShadowSplits[splitIndex].Proj);
	float2 shadowMapCoord = 0.5f + float2(0.5f, -0.5f) * shadowCoord.xy;

	clip( float4(shadowMapCoord.xy, 1.0f - shadowMapCoord.xy) );

	float shadowRange = DirectionalLight.ShadowSplits[splitIndex].NearFar.y - DirectionalLight.ShadowSplits[splitIndex].NearFar.x;

	float shadowDepth = DirectionalLightShadowMaps.SampleLevel(ShadowSampler, float3(shadowMapCoord, splitIndex), 0.0f).r;
	float shadowDeltaDepth = (shadowCoord.z - shadowDepth * shadowCoord.w) * shadowRange;
	clip(0.5f + shadowDeltaDepth);

	float3 shadowMapCoordZ = float3(shadowMapCoord, shadowCoord.z);
	
	float3 shadowMapCoordDDX = ddx(shadowMapCoordZ), shadowMapCoordDDY = ddy(shadowMapCoordZ);
	float2 shadowMapCoordDepthCorrection = float2(
		shadowMapCoordDDX.y * shadowMapCoordDDY.z - shadowMapCoordDDY.y * shadowMapCoordDDX.z,
		shadowMapCoordDDY.x * shadowMapCoordDDX.z - shadowMapCoordDDX.x * shadowMapCoordDDY.z )
		/ (shadowMapCoordDDY.x * shadowMapCoordDDX.y - shadowMapCoordDDX.x * shadowMapCoordDDY.y);

	float scaledRadius = 0.05f + 0.2f * clamp(0.2f * shadowDeltaDepth, 0.0f, 5.0f);
	float2 scaledOffset = scaledRadius * DirectionalLight.ShadowSplits[splitIndex].PixelScale; // lerp( 2.0f, 0.25f, 1.0f / (1.0f + 0.02f * eyeDepth) )

	float shadowMultiplier = 50.0f;

	float testVisibility = saturate( 1.0f - shadowMultiplier * shadowDeltaDepth );
	float visibility = 0.0f;

	float4 noise = NoiseTexture.SampleLevel(NoiseSampler, p.Position.xy / 32.0f, 0) * 2.0f - 1.0f;

	float depthCorrectionAcc = 0.0f;

	for (int i = 0; i < 8; ++i)
	{
		float2 sampleOffset = scaledOffset * (PoissonKernel[i].x * noise.xy + PoissonKernel[i].y * noise.zw);

		float sampleDepthCorrection = min(0.0f, dot(sampleOffset, shadowMapCoordDepthCorrection));

		depthCorrectionAcc -= sampleDepthCorrection;

		float sampleDepth = DirectionalLightShadowMaps.SampleLevel(ShadowSampler, float3(shadowMapCoord + sampleOffset, splitIndex), 0.0f).r;
		float sampleDeltaDepth = (shadowCoord.z - sampleDepth * shadowCoord.w) * shadowRange + sampleDepthCorrection;
		visibility += saturate( 1.0f - shadowMultiplier * sampleDeltaDepth );
	}

	visibility = saturate( 2.5 * visibility / 8 );
	
	return visibility;
}

float4 PSMain(Pixel p, uniform bool bShadowed = true) : SV_Target0
{
	float4 eyeGeometry = BE_SCENE_TEXTURE(SceneGeometryTexture).SampleLevel(DefaultSampler, p.TexCoord, 0);
	float4 diffuseColor = BE_SCENE_TEXTURE(SceneDiffuseTexture).SampleLevel(DefaultSampler, p.TexCoord, 0);
	
//	return diffuseColor;

	float3 worldNormal = ExtractNormal(eyeGeometry);

	float4 intensity = 1.0f;

	if (bShadowed)
		intensity = ShadowTexture.SampleLevel(DefaultSampler, p.TexCoord, 0);
	
	// Angle fallof
	float cosAngle = dot(eyeGeometry.yzw, -DirectionalLight.Dir);
	float negIntensity = saturate(0.5f - 0.1f * cosAngle) * AmbientTexture.SampleLevel(DefaultSampler, p.TexCoord, 0).a;
	float4 posIntensity = saturate(cosAngle) * intensity;
	
	float roundFactor = frac(diffuseColor.a);
	intensity = lerp(posIntensity, negIntensity, roundFactor);
	
	return float4(diffuseColor.xyz * DirectionalLight.Color.xyz * intensity.xyz, 0.0f);
}

technique11 Shadowed <
	bool EnableProcessing = true;
	string PipelineStage = "LightingPipelineStage";
	string RenderQueue = "DefaultRenderQueue";
>
{
	pass <
		string Color0 = "ShadowTarget";
		float4 ClearColor0 = float4(1.0f, 1.0f, 1.0f, 1.0f);
		bool bClearColor0 = true;

		string LightType = "DirectionalLight";
		bool Shadowed = true;
	>
	{
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );

		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSShadow()) );
	}
	
	pass <
		string Color0 = "ShadowTarget";
		
		string LightType = "DirectionalLight";
		bool Shadowed = true;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSBilateralAverage(
			float2(1.0f, 0.0f), 3, 4,
			BE_SCENE_TEXTURE(SceneGeometryTexture), DefaultSampler,
			ShadowTexture, ShadowSampler, DestinationResolution.zw )) );
	}

	pass <
		string Color0 = "ShadowTarget";
		
		string LightType = "DirectionalLight";
		bool Shadowed = true;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSBilateralAverage(
			float2(0.0f, 1.0f), 3, 4,
			BE_SCENE_TEXTURE(SceneGeometryTexture), DefaultSampler,
			ShadowTexture, ShadowSampler, DestinationResolution.zw )) );
	}
	
	pass <
		string Color0 = "SceneTarget";
		bool bKeepColor0 = true;

		string LightType = "DirectionalLight";
		bool Shadowed = true;

		bool RevertBlending = true;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSMain(true)) );
	}
}

technique11 Default <
	string PipelineStage = "LightingPipelineStage";
	string RenderQueue = "DefaultRenderQueue";
>
{
	pass < string LightType = "DirectionalLight"; bool Shadowed = false; >
	{
		SetVertexShader( CompileShader(vs_4_0, VSMain()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSMain(false)) );
	}
}
