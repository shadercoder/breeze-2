#ifndef DRAGTREEWIDGET_H
#define DRAGTREEWIDGET_H

#include "breezEd.h"
#include <QtGui/QAction>

#include "Checked.h"

/// Allows only one action in a group to be checked at a time.
class ExclusiveActionGroup : public QObject
{
	Q_OBJECT

private:
	QList<QAction*> m_actions;

private Q_SLOTS:
	/// A different(?) action was checked.
	void actionChecked()
	{
		// Check action
		QAction *pAction = qobject_cast<QAction*>(sender());
		pAction->setChecked(true);

		// Uncheck all other actions
		Q_FOREACH (QAction *pOther, m_actions)
			if (pOther != pAction)
				pOther->setChecked(false);
	}

public:
	/// Constructor.
	ExclusiveActionGroup(QWidget *pParent = nullptr)
		: QObject(pParent) { }

	/// Adds the given action to this group.
	void addAction(QAction *pAction)
	{
		m_actions.push_back( LEAN_ASSERT_NOT_NULL(pAction) );
		checkedConnect(pAction, SIGNAL(triggered()), this, SLOT(actionChecked()));
	}
};

#endif