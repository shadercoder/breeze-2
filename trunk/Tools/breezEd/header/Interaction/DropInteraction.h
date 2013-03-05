#ifndef DROP_INTERACTION_H
#define DROP_INTERACTION_H

#include "breezEd.h"
#include "Interaction.h"

class QDropEvent;

/// Interaction interface.
class DropInteraction : public Interaction
{
protected:
	DropInteraction& operator =(const DropInteraction&) { return *this; }
	~DropInteraction() { }

public:
	/// Accepts the given drop event, if matching.
	virtual void accept(QDropEvent &dropEvent) = 0;
	/// Called when a drag & drop operation is cancelled.
	virtual void cancel() { }
	/// Called when a drag & drop operation is completed.
	virtual void complete(QDropEvent &dropEvent) { }
};

/// Creates drag/drop mime data for the given interaction.
class QMimeData* interactionMimeData(DropInteraction *interaction);
/// Retrieves a drop interaction.
DropInteraction* interactionFromMimeData(const QMimeData *data);

#endif
