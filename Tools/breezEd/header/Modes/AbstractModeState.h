#ifndef ABSTRACTMODESTATE_H
#define ABSTRACTMODESTATE_H

#include "breezEd.h"
#include <QtCore/QObject.h>

class Mode;

/// Abstract mode state interface.
class AbstractModeState : public QObject
{
	Q_OBJECT

friend class Mode;

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent) = 0;
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent) = 0;

public:
	/// Constructor.
	AbstractModeState(QObject *pParent = nullptr)
		: QObject(pParent) { }
};

#endif
