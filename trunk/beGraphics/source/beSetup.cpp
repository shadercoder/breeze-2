/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/beSetup.h"

namespace beGraphics
{

// Transfers all data from the given source setup to the given destination setup.
void Transfer(Setup &dest, const Setup &source)
{
	TransferProperties(dest, source);
	TransferTextures(dest, source);
}

} // namespace
