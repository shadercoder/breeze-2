#include "stdafx.h"
#include "Interaction/SelectInteraction.h"

#include "Utility/InputProvider.h"

#include "Interaction/Queries.h"
#include "Documents/SceneDocument.h"

#include <lean/logging/errors.h>

// Constructor.
SelectInteraction::SelectInteraction(SceneDocument *pDocument, QObject *pParent)
	: QObject( pParent ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) )
{
}

// Destructor.
SelectInteraction::~SelectInteraction()
{
}

// Steps the interaction.
void SelectInteraction::step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective)
{
	if (input.buttonPressed(Qt::LeftButton, true))
	{
		beEntitySystem::Entity *pEntity = bees::FirstAccessibleEntity( entityUnderCursor(*m_pDocument, input.relativePosition(), perspective) );
		
		bool bMulti = input.keyPressed(Qt::Key_Shift);
		bool bToggle = input.keyPressed(Qt::Key_Control);

		if (bMulti || bToggle)
		{
			if (pEntity)
			{
				SceneDocument::EntityVector selection = m_pDocument->selection();
				int idx = selection.indexOf(pEntity);

				if (idx == -1)
					selection.push_back(pEntity);
				else if (bToggle)
					selection.remove(idx);

				m_pDocument->setSelection(selection);
			}
		}
		else
			m_pDocument->setSelection(pEntity);

		input.setButtonHandled(Qt::LeftButton);
	}
}
