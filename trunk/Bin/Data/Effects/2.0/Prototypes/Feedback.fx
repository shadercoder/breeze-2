#define BE_RENDERABLE_INCLUDE_PROJ
#define BE_RENDERABLE_INCLUDE_ID

#include <Engine/Perspective.fx>
#include <Engine/Renderable.fx>

struct Vertex
{
	float4 Position	: Position;
};

float4 VSObjectIDs(Vertex v) : SV_Position
{
	return mul(v.Position, WorldViewProj);
}

uint4 PSObjectIDs(float4 p : SV_Position) : SV_Target0
{
	return ObjectID;
}

technique11 ObjectIDs <
	string PipelineStage = "ObjectIDPipelineStage";
	string RenderQueue = "DefaultRenderQueue";
>
{
	pass
	{
		SetVertexShader( CompileShader(vs_4_0, VSObjectIDs()) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_4_0, PSObjectIDs()) );
	}
}
