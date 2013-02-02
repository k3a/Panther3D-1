//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (22.10.2006)
// Purpose:	Sending messages to console - entity
//-----------------------------------------------------------------------------
#include "eTarget_console.h"

REGISTER_ENTITY(eTarget_console, "target_console", false);

eTarget_console::eTarget_console()
{
	ENT_DATA("commands", m_strCmds, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "command param;command;var value");
}

bool eTarget_console::Spawn(geometryData_s& gdata)
{
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	return !!g_pConsole;
}

void eTarget_console::DoInternalCmds()
{
	if (!m_strCmds || m_strCmds[0]==0) 
	{
		ECON(MSG_CON_DEBUG, "Action(): entity don't have commands key specified!");
		return; // neni co provadet
	}
	
	int lastCommandStrIndex=0; char tmpCmd[256]="";
	for (UINT i=0;i<strlen(m_strCmds);i++)
	{
		if (m_strCmds[i]==';')
		{
			// neni moc dlouhy?
			if(i - lastCommandStrIndex > 255)
			{
				ECON(MSG_CON_DEBUG, "Some command in commands key too long!");
				return;
			}
			// ziskej prikaz
			strncpy(tmpCmd, &m_strCmds[lastCommandStrIndex], i - lastCommandStrIndex);
			// proved ho
			g_pConsole->Command(tmpCmd);
			// nastav novy pocatcni index
			lastCommandStrIndex = i+1;
		}
	}
	
	// posledni prikaz nebo jeden prikaz bez stredniku
	g_pConsole->Command(&m_strCmds[lastCommandStrIndex]);
}

bool eTarget_console::Command(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams)
{
	ECMDLIST_BEGIN
		ECMD_BEGIN(_EC_COMMAND)
			g_pConsole->Command(params[0].c);
		ECMD_END
		ECMD_BEGIN(_EC_ACTION)
			DoInternalCmds();
		ECMD_END
	ECMDLIST_END
}