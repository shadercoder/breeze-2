#ifndef UNDO_H
#define UNDO_H

#include "breezEd.h"
#include <QtGui/QUndoStack>

/// Undos the given command, if it still is the one most recently excuted.
inline void undoIfMostRecent(QUndoStack &stack, const QUndoCommand &command)
{
	int undoIndex = stack.index();

	// Make sure command still most recent
	if (undoIndex > 0 && stack.command(undoIndex - 1) == &command)
		stack.undo();
}

/// Redoes the given command, if it still is the one most recently undone.
inline void redoIfMostRecent(QUndoStack &stack, const QUndoCommand &command)
{
	int undoIndex = stack.index();

	// Make sure command still most recent
	if (stack.command(undoIndex) == &command)
		stack.redo();
}

#endif