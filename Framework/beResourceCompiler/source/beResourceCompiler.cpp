// beResourceCompiler.cpp : Defines the exported functions for the DLL application.
//

#include "beResourceCompilerInternal/stdafx.h"
#include "beResourceCompiler/beResourceCompiler.h"

// Opens a message box containing version information.
void beResourceCompiler::InfoBox()
{
	::MessageBoxW(NULL, L"beResourceCompiler build " _T(__TIMESTAMP__) L".", L"Version info", MB_OK);
}
