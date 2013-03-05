// beNature.cpp : Defines the exported functions for the DLL application.
//

#include "beAssetsInternal/stdafx.h"
#include "beAssets/beAssets.h"

// NOP, may be used to enforce module linkage.
void beAssets::Link()
{
}

// Opens a message box containing version information.
void beAssets::InfoBox()
{
	::MessageBoxW(NULL, L"beAssets build " _T(__TIMESTAMP__) L".", L"Version info", MB_OK);
}
