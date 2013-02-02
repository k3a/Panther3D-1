//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (31.7.2006,13.1.2006)
// Purpose:	Error handling and reporting functions
//-----------------------------------------------------------------------------
#pragma once

#define _WIN32_WINNT 0x0400 // pro IsDebuggerPresent ktere funguje pouze ve Windows 98+
#include <windows.h>
#include <dbghelp.h>

int HandleException(EXCEPTION_POINTERS* pExceptionPointers);
int GenerateMinidump(EXCEPTION_POINTERS* pExceptionPointers, char* dmpFileName);