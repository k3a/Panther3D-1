//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Definition of common variables and functions
//-----------------------------------------------------------------------------

#include "common.h"

IP3DConsole	*g_pConsole; // global console
IP3DFileSystem	*g_pFS;
IP3DXML	*g_pXML;

ConVar	CVs_soundfx("s_soundfx", "1", CVAR_NOT_STRING | CVAR_ARCHIVE, "Aktivovat zvukove efekty");