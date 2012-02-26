#include "stdafx.h"
#include "Commands/SetComponent.h"

#include <QtCore/QCoreApplication>
#include "Utility/Strings.h"

// Constructor.
SetComponentCommand::SetComponentCommand(beCore::ReflectedComponent *pReflectedComponent, uint4 componentIdx,
		const lean::any &component, QUndoCommand *pParent)
	: QUndoCommand(
		QCoreApplication::translate("SetComponentCommand", "Replaced component '%1'").arg(
				makeName(toQt( LEAN_ASSERT_NOT_NULL(pReflectedComponent)->GetComponentName(componentIdx) ))
			),
		pParent ),
	m_pReflectedComponent( LEAN_ASSERT_NOT_NULL(pReflectedComponent) ),
	m_componentIdx( componentIdx ),
	m_previousComponent( *pReflectedComponent->GetComponent(m_componentIdx) ),
	m_component( component )
{
}

// Destructor.
SetComponentCommand::~SetComponentCommand()
{
}

// Reverts the property.
void SetComponentCommand::undo()
{
	m_pReflectedComponent->SetComponent(m_componentIdx, m_previousComponent);
}

// Changes the property.
void SetComponentCommand::redo()
{
	m_pReflectedComponent->SetComponent(m_componentIdx, m_component);
}
