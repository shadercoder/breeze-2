#include "stdafx.h"
#include "Modes/ParallelMode.h"

#include "Modes/Mode.h"

// Constructor.
ParallelMode::ParallelMode(Mode *pMode, QObject *pParent)
	: AbstractModeState(pParent),
	m_pParallelMode( LEAN_ASSERT_NOT_NULL(pMode) )
{
	m_pParallelMode->setParent(this);
}

// Destructor.
ParallelMode::~ParallelMode()
{
}

// Called when the mode is entered.
void ParallelMode::onEntry(QEvent *pEvent)
{
	m_pParallelMode->enter();
}

// Called when the mode is exited.
void ParallelMode::onExit(QEvent *pEvent)
{
	m_pParallelMode->exit();
}
