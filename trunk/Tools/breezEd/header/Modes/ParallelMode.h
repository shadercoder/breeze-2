#ifndef PARALLELMODE_H
#define PARALLELMODE_H

#include "breezEd.h"
#include "AbstractModeState.h"

/// Parallel mode class
class ParallelMode : public AbstractModeState
{
	Q_OBJECT

private:
	Mode *m_pParallelMode;

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent);

public:
	/// Constructor.
	ParallelMode(Mode *pMode, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~ParallelMode();
};

#endif
