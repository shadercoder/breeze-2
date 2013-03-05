#include "stdafx.h"
#include "Interaction/TranslateInteraction.h"

#include "Commands/TranslateEntity.h"

#include "Utility/InputProvider.h"

#include "Interaction/Queries.h"
#include "Documents/SceneDocument.h"

#include "DeviceManager.h"

#include <beEntitySystem/beEntities.h>
#include <beEntitySystem/beWorldControllers.h>
#include "Interaction/Widgets.h"
#include "Interaction/Math.h"

#include "Operations/EntityOperations.h"

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

#include "Utility/Checked.h"

#include <lean/logging/errors.h>

// Constructor.
TranslateInteraction::TranslateInteraction(SceneDocument *pDocument, QObject *pParent)
	: QObject( pParent ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pCommand(),
	m_axisID( InvalidAxisID )
{
	// TODO: NO HARD-CODED PATHS
	m_axes[0] = createWidgetMesh("Static/UI/TranslateWidget.mesh", beMath::vec(0.7f, 0.0f, 0.0f, 0.5f), "",
		*m_pDocument->world()->Entities(), *m_pDocument->world()->Controllers().GetController<besc::MeshControllers>(),
		*m_pDocument->graphicsResources(), *m_pDocument->renderer());
	m_axes[1] = createWidgetMesh("Static/UI/TranslateWidget.mesh", beMath::vec(0.0f, 0.7f, 0.0f, 0.5f), "",
		*m_pDocument->world()->Entities(), *m_pDocument->world()->Controllers().GetController<besc::MeshControllers>(),
		*m_pDocument->graphicsResources(), *m_pDocument->renderer());
	m_axes[2] = createWidgetMesh("Static/UI/TranslateWidget.mesh", beMath::vec(0.0f, 0.0f, 0.7f, 0.5f), "",
		*m_pDocument->world()->Entities(), *m_pDocument->world()->Controllers().GetController<besc::MeshControllers>(),
		*m_pDocument->graphicsResources(), *m_pDocument->renderer());
}

// Destructor.
TranslateInteraction::~TranslateInteraction()
{
}

// Steps the interaction.
void TranslateInteraction::step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective)
{
	using namespace beMath;

	// NOTE: Make sure widgets are up to date *before* editing has started
	updateWidgets();

	uint4 axisID = InvalidAxisID;

	// Keep / stop moving
	if (!m_selection.empty() && input.buttonPressed(Qt::LeftButton))
	{
		axisID = m_axisID;

		// Check if user picked new axis
		if (axisID == InvalidAxisID && input.buttonPressed(Qt::LeftButton, true))
		{
			uint4 objectID = objectIDUnderCursor(*m_pDocument->renderer(), *m_pDocument->scene(), input.relativePosition(), perspective);

			for (int i = 0; i < 3; ++i)
				if (objectID == m_axes[i]->GetCustomID())
					axisID = i;

			if (axisID != InvalidAxisID && input.keyPressed(Qt::Key_Control))
			{
				// IMPORTANT: Clone selection, will change on duplication!
				entity_vector selection = m_selection;
				DuplicateEntities(&selection[0], (uint4) selection.size(), m_pDocument);
			}
		}

		// Moving
		if (axisID != InvalidAxisID)
			input.setButtonHandled(Qt::LeftButton);
	}
	else
		finishEditing();

	if (axisID < 3)
	{
		fvec3 axisOrig = (m_axisID != InvalidAxisID) ? m_axisStop : m_axes[axisID]->GetPosition();
		fvec3 axisDir = (m_axisID != InvalidAxisID) ? m_axisDir : m_axes[axisID]->GetOrientation()[2];

		fvec3 rayOrig, rayDir;
		camRayDirUnderCursor(rayOrig, rayDir, toQt(input.relativePosition()), perspective.ViewProjMat);

		// Compute point of closest crossing
		matrix<float, 3, 2> intersectMat = mat_col(rayDir, -axisDir);
		matrix<float, 2, 3> intersectMatT = transpose(intersectMat);
		float axisDelta = mul( inverse(mul(intersectMatT, intersectMat)), mul(intersectMatT, axisOrig - rayOrig) )[1];

		fvec3 nextAxisStop = axisOrig + axisDir * axisDelta;

		// Perform move operation
		if (m_axisID != InvalidAxisID)
		{
			fvec3 moveDelta = nextAxisStop - m_axisStop;
			
			for (entity_vector::const_iterator itEntity = m_selection.begin(); itEntity != m_selection.end(); ++itEntity)
			{
				(*itEntity)->SetPosition( (*itEntity)->GetPosition() + moveDelta );
				(*itEntity)->NeedSync();
			}
			
			m_pCommand->capture();
			updateWidgets();
		}
		// Initialize move operation
		else
		{
			m_pCommand = new TranslateEntityCommand(m_selection);
			m_pDocument->undoStack()->push(m_pCommand);

			m_axisID = axisID;
		}

		m_axisStop = nextAxisStop;
		m_axisDir = axisDir;
	}
}

// Attaches this interaction.
void TranslateInteraction::attach()
{
	for (int i = 0; i < 3; ++i)
		m_axes[i]->Attach();

	checkedConnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(selectionChanged()));
}

// Detaches this interaction.
void TranslateInteraction::detach()
{
	for (int i = 0; i < 3; ++i)
		m_axes[i]->Detach();

	disconnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(selectionChanged()));
}

// Finish editing.
void TranslateInteraction::finishEditing()
{
	m_pCommand = nullptr;
	m_axisID = InvalidAxisID;
}

// Update widgets.
void TranslateInteraction::selectionChanged()
{
	finishEditing();
	m_selection = m_pDocument->selection();
	updateWidgets();
}

// Update widgets.
void TranslateInteraction::updateWidgets()
{
	beMath::fmat3 orientation = (m_selection.size() == 1)
		? m_selection.front()->GetOrientation()
		: beMath::mat_diag<3, 3>(1.0f);
	
	m_centroid = 0.0f;

	if (!m_selection.empty())
	{
		for (entity_vector::const_iterator itEntity = m_selection.begin(); itEntity != m_selection.end(); ++itEntity)
			m_centroid += (*itEntity)->GetPosition();

		m_centroid /= m_selection.size();
	}

	for (int i = 0; i < 3; ++i)
	{
		m_axes[i]->SetPosition(m_centroid);
		m_axes[i]->SetOrientation(
				beMath::mat_row(
					orientation[ (i + 1) % 3 ],
					orientation[ (i + 2) % 3 ],
					orientation[ (i + 0) % 3 ]
				)
			);

		m_axes[i]->SetVisible(!m_selection.empty());
	}
}
