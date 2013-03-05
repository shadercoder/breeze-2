#include "stdafx.h"
#include "Interaction/DropInteraction.h"

#include "Utility/MimeData.h"

// Creates drag/drop mime data for the given interaction.
QMimeData* interactionMimeData(DropInteraction *interaction)
{
	return ptrToMimeData("application/breezed-interaction", interaction);
}

// Retrieves a drop interaction.
DropInteraction* interactionFromMimeData(const QMimeData *pData)
{
	return ptrFromMimeData<DropInteraction>("application/breezed-interaction", pData);
}
