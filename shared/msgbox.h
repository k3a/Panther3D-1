//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Jednoduchý MesssageBox s více parametry
//-----------------------------------------------------------------------------
#pragma once

#include <windows.h>
#include <stdio.h>

int MsgBox(HWND hWnd, unsigned int uType, const char* strTitle, const char* strText, ...);