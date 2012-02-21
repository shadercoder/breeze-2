// bePhysics.cpp : Defines the exported functions for the DLL application.
//

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/bePhysics.h"

// Opens a message box containing version information.
void bePhysics::InfoBox()
{
	::MessageBoxW(NULL, L"bePhysics build " _T(__TIMESTAMP__) L".", L"Version info", MB_OK);
}
