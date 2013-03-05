#include "stdafx.h"
#include "Interaction/Queries.h"

#include "Documents/SceneDocument.h"
#include <beScene/bePipePool.h>
#include <beScene/bePipe.h>
#include <beScene/bePerspectivePool.h>
#include <beScene/beRenderingPipeline.h>

#include <beEntitySystem/beEntities.h>

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

#include <lean/logging/errors.h>

/// Retrieve the object ID under the given cursor position.
uint4 objectIDUnderCursor(beScene::Renderer &renderer, beScene::RenderingController &scene, const QPointF &relativePos, const beScene::PerspectiveDesc &perspectiveDesc)
{
	using namespace beMath;

	beScene::PerspectiveDesc rayPerspective(perspectiveDesc);

	// Offset camera to ray perspective
	fmat4 rayOffsetScale = fmat4::identity;
	rayOffsetScale[3][0] -= 2 * relativePos.x() - 1;
	rayOffsetScale[3][1] -= -2 * relativePos.y() + 1;
	rayOffsetScale = mul( rayOffsetScale, mat_scale<4>(10000.0f, 10000.0f, 1.0f) );

	rayPerspective.ProjMat = mul( rayPerspective.ProjMat, rayOffsetScale );
	rayPerspective.ViewProjMat = mul( rayPerspective.ViewMat, rayPerspective.ProjMat );
	rayPerspective.OutputIndex = 0;

	uint4 objectIDStage = renderer.Pipeline()->GetStageID("ObjectIDPipelineStage");

	if (objectIDStage == beScene::InvalidPipelineStage)
	{
		LEAN_LOG_ERROR_MSG("Pipeline stage 'ObjectIDPipelineStage' unavailable, cannot perform entity query.");
		return beEntitySystem::Entity::InvalidID;
	}

	// Acquire selection pipe
	beGraphics::TextureTargetDesc selectionPipeDesc(1, 1, 1, beGraphics::Format::R32U, beGraphics::SampleDesc(1));
	lean::resource_ptr<beScene::Pipe> pSelectionPipe = renderer.PipePool()->GetPipe(selectionPipeDesc);
	pSelectionPipe->KeepResults();

	// Perform scene query
	lean::com_ptr<besc::PipelinePerspective> perspective = renderer.PerspectivePool()->GetPerspective(
		rayPerspective, pSelectionPipe, nullptr, 1U << objectIDStage);
	scene.Render(*perspective, *scene.GetRenderContext());

	const beGraphics::ColorTextureTarget *pObjectIDTarget = pSelectionPipe->GetColorTarget("ObjectIDTarget");

	// Nothing rendered
	if (!pObjectIDTarget)
		return beEntitySystem::Entity::InvalidID;
	
	uint4 selectedObjectID = beEntitySystem::Entity::InvalidID;
	
	// Read back object IDs
	renderer.TargetPool()->ReadBack(
			pObjectIDTarget, &selectedObjectID, sizeof(selectedObjectID),
			*renderer.ImmediateContext()
		);

	// Release pipe resources
	pSelectionPipe->KeepResults(false);
	pSelectionPipe->Release();

	return selectedObjectID;
}

/// Retrieve the entity under the given cursor position.
beEntitySystem::Entity* entityUnderCursor(SceneDocument &document, const QPointF &relativePos, const beScene::PerspectiveDesc &perspective)
{
	uint4 selectedObjectID = objectIDUnderCursor(*document.renderer(), *document.scene(), relativePos, perspective);
	
	return (selectedObjectID != beEntitySystem::Entity::InvalidID)
		? document.world()->Entities()->GetEntityByCustomID(selectedObjectID)
		: nullptr;
}
