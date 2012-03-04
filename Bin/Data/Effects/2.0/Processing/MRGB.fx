#define BE_SCENE_SETUP

#include <Engine/Pipe.fx>
#include <Pipelines/LPR/Scene.fx>
#include <Pipelines/LPR/Geometry.fx>
#include "Processing/MRGeometry.fx"

float4 SceneGeometryResolution : SceneGeometryTargetResolution;
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

float max4(float4 value)
{
	value.xy = max(value.xy, value.zw);
	return max(value.x, value.y);
}

float min4(float4 value)
{
	value.xy = min(value.xy, value.zw);
	return min(value.x, value.y);
}

float4 PSDownsample(Pixel p, uniform Texture2D sourceTex, uniform float2 sourcePixel, uniform int sourceLevel = 0) : SV_Target0
{
	float4 eyeDepths;
	float3 eyeNormal = 0.0f;

	int eyeDepthCount = 0;

	// Sample source box
	[unroll] for (float2 sampleOffset = -0.5f; sampleOffset.y < 1.0f; sampleOffset.y += 1.0f)
		[unroll] for (sampleOffset.x = -0.5f; sampleOffset.x < 1.0f; sampleOffset.x += 1.0f)
		{
			float4 sampleGeometry = sourceTex.SampleLevel(DefaultSampler, p.TexCoord + sampleOffset * sourcePixel, sourceLevel);

			// Store depths separately
			eyeDepths[eyeDepthCount++] = ExtractDepth(sampleGeometry);

			// Sum up normals
			eyeNormal += ExtractNormal(sampleGeometry);
		}

	// Compute median
	float eyeDepth = dot(eyeDepths, 1);
	eyeDepth -= min4(eyeDepths);
	eyeDepth -= max4(eyeDepths);
	eyeDepth *= 0.5f;

	// Average normals
	eyeNormal = normalize(eyeNormal);

	return BuildGeometry(eyeDepth, eyeNormal);
}

technique11 Default <
	bool DontFlip = true;
>
{
	pass <
		string Color0 = "SceneGeometryTarget2";
		float ScaleX = 1.0f / 2;
		float ScaleY = 1.0f / 2;
	>
	{
		SetRasterizerState( NULL );
		SetDepthStencilState( NULL, 0 );
		SetBlendState( NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff );
		
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSDownsample(SceneGeometryTexture, SceneGeometryResolution.zw)) );
	}
	
	pass <
		string Color0 = "SceneGeometryTarget4";
		float ScaleX = 1.0f / 4;
		float ScaleY = 1.0f / 4;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSDownsample(SceneGeometryTexture2, SceneGeometryResolution.zw / 2)) );
	}

	pass <
		string Color0 = "SceneGeometryTarget8";
		float ScaleX = 1.0f / 8;
		float ScaleY = 1.0f / 8;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSDownsample(SceneGeometryTexture4, SceneGeometryResolution.zw / 4)) );
	}

	pass <
		string Color0 = "SceneGeometryTarget16";
		float ScaleX = 1.0f / 16;
		float ScaleY = 1.0f / 16;
	>
	{
		SetVertexShader( CompileShader(vs_4_0, VSQuad()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSDownsample(SceneGeometryTexture8, SceneGeometryResolution.zw / 8)) );
	}
}
