/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PASS_SEQUENCE
#define BE_SCENE_PASS_SEQUENCE

#include "beScene.h"

namespace beScene
{

/// Multi-pass effect binder interface.
template <class Pass>
class PassSequence
{
protected:
	LEAN_INLINE PassSequence& operator =(const PassSequence&) { return *this; }
	LEAN_INLINE ~PassSequence() throw() { }

public:
	/// Pass type.
	typedef Pass PassType;

	/// Gets the number of passes.
	virtual uint4 GetPassCount() const = 0;
	/// Gets the pass identified by the given ID.
	virtual const PassType* GetPass(uint4 passID) const = 0;
};

} // namespace

#endif