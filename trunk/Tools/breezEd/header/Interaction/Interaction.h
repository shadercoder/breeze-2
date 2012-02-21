#ifndef INTERACTION_H
#define INTERACTION_H

#include "breezEd.h"
#include <beScene/bePerspective.h>

class InputProvider;

/// Interaction interface.
class Interaction
{
protected:
	Interaction& operator =(const Interaction&) { return *this; }
	~Interaction() { }

public:
	/// Attaches this interaction.
	virtual void attach() { }
	/// Detaches this interaction.
	virtual void detach() { }

	/// Steps the interaction.
	virtual void step(float timeStep, InputProvider &input, const beScene::PerspectiveDesc &perspective) = 0;
};

#endif
