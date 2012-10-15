#include "stdafx.h"
#include "Interaction/ScaleInteraction.h"

#include "Commands/ScaleEntity.h"

#include "Utility/InputProvider.h"

#include "Interaction/Queries.h"
#include "Documents/SceneDocument.h"

#include "DeviceManager.h"

#include <beEntitySystem/beEntity.h>
#include "Interaction/Widgets.h"
#include "Interaction/Math.h"

#include <beMath/beVector.h>
#include <beMath/beMatrix.h>

#include "Utility/Checked.h"

#include <lean/logging/errors.h>

static const uint4 ScaleAxisWidgetIDs[3] = { reserveWidgetID(), reserveWidgetID(), reserveWidgetID() };

// Constructor.
ScaleInteraction::ScaleInteraction(SceneDocument *pDocument, QObject *pParent)
	: QObject( pParent ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pCommand(),
	m_axisID( InvalidAxisID )
{
	// TODO: NO HARD-CODED PATHS
	m_axes[0] = createWidgetMesh(ScaleAxisWidgetIDs[0], "Static/UI/ScaleWidget.mesh", beMath::vec(1.0f, 0.1f, 0.0f, 0.7f),
		m_pDocument->scene(), *m_pDocument->deviceManager()->graphicsResources(), *m_pDocument->renderer());
	m_axes[1] = createWidgetMesh(ScaleAxisWidgetIDs[1], "Static/UI/ScaleWidget.mesh", beMath::vec(0.0f, 1.0f, 0.1f, 0.7f),
		m_pDocument->scene(), *m_pDocument->deviceManager()->graphicsResources(), *m_pDocument->renderer());
	m_axes[2] = createWidgetMesh(ScaleAxisWidgetIDs[2], "Static/UI/ScaleWidget.mesh", beMath::vec(0.1f, 0.0f, 1.0f, 0.7f),
		m_pDocument->scene(), *m_pDocument->deviceManager()->graphicsResources(), *m_pDocument->renderer());
}

// Destructor.
ScaleInteraction::~ScaleInteraction()
{
}

// Steps the interaction.
void ScaleInteraction::step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective)
{
	using namespace beMath;

	// TODO: Change-based update?
	updateWidgets();

	uint4 axisID = InvalidAxisID;

	// Keep / stop scaling
	if (input.buttonPressed(Qt::LeftButton))
	{
		axisID = m_axisID;

		// Check if user picked new axis
		if (axisID == InvalidAxisID && input.buttonPressed(Qt::LeftButton, true))
		{
			uint4 objectID = objectIDUnderCursor(*m_pDocument->renderer(), *m_pDocument->scene(), input.relativePosition(), perspective);

			for (int i = 0; i < 3; ++i)
				if (objectID == ScaleAxisWidgetIDs[i])
					axisID = i;
		}

		// Scaling
		if (axisID != InvalidAxisID)
			input.setButtonHandled(Qt::LeftButton);
	}
	else
		finishEditing();

	if (axisID < 3)
	{
		// NOTE: Make sure widgets are up to date *before* editing has started
		if (m_axisID == InvalidAxisID)
			updateWidgets();

		fvec3 axisOrig = m_axes[axisID]->GetPosition();
		fvec3 axisDir = m_axes[axisID]->GetOrientation()[2];

		fvec3 rayOrig, rayDir;
		camRayDirUnderCursor(rayOrig, rayDir, toQt(input.relativePosition()), perspective.ViewProjMat);

		// Compute point of closest crossing
		matrix<float, 3, 2> intersectMat = mat_col(rayDir, -axisDir);
		matrix<float, 2, 3> intersectMatT = transpose(intersectMat);
		float axisDelta = mul( inverse(mul(intersectMatT, intersectMat)), mul(intersectMatT, axisOrig - rayOrig) )[1];

		fvec3 nextAxisStop = axisOrig + axisDir * axisDelta;

		// Perform sacle operation
		if (m_axisID != InvalidAxisID)
		{
			float scaleFactor = dot(m_axisStop - axisOrig, axisDir);
			float nextScaleFactor = dot(nextAxisStop - axisOrig, axisDir);

			if (abs(scaleFactor) > 0.001f && abs(nextScaleFactor) > 0.001f)
			{
				float scaleDelta = nextScaleFactor / scaleFactor;
				
				for (SceneDocument::EntityVector::const_iterator itEntity = m_selection.begin(); itEntity != m_selection.end(); ++itEntity)
				{
//					fvec3 position = (*itEntity)->GetPosition();
					fvec3 scaling = (*itEntity)->GetScaling();
					
//					position = (position - m_centroid) * scaleDelta + m_centroid;
					scaling[m_axisID] *= scaleDelta;
					
//					(*itEntity)->SetPosition(position);
					(*itEntity)->SetScaling(scaling);
					(*itEntity)->Synchronize();
				}
				
				m_axisStop = nextAxisStop;
				m_pCommand->capture();
				updateWidgets();
			}
		}
		// Initialize scale operation
		else
		{
			m_pCommand = new ScaleEntityCommand(m_selection);
			m_pDocument->undoStack()->push(m_pCommand);

			m_axisStop = nextAxisStop;
			m_axisID = axisID;
		}
	}
}

// Attaches this interaction.
void ScaleInteraction::attach()
{
	for (int i = 0; i < 3; ++i)
		m_axes[i]->Attach();

	checkedConnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(selectionChanged()));
}

// Detaches this interaction.
void ScaleInteraction::detach()
{
	for (int i = 0; i < 3; ++i)
		m_axes[i]->Detach();

	disconnect(m_pDocument, SIGNAL(selectionChanged(SceneDocument*)), this, SLOT(selectionChanged()));
}

// Finish editing.
void ScaleInteraction::finishEditing()
{
	m_pCommand = nullptr;
	m_axisID = InvalidAxisID;
}

// Update widgets.
void ScaleInteraction::selectionChanged()
{
	finishEditing();
	m_selection = m_pDocument->selection();
	updateWidgets();
}

// Update widgets.
void ScaleInteraction::updateWidgets()
{
	beMath::fmat3 orientation = (m_selection.size() == 1) ? m_selection.front()->GetOrientation() : beMath::mat_diag<3, 3>(1.0f);
	
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
