//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (22.10.2006)
// Purpose:	Sending messages to console - entity
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eTarget_console : public eBase
{
public:
	eTarget_console();
	void DoInternalCmds();
	bool Spawn(geometryData_s& gdata);
	bool Command(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams);
private:
	char* m_strCmds;
};