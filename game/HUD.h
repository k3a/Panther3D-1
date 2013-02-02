//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose: Game HUD
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dfont.h"
#include "common.h"

class CHUD
{
public:
	CHUD();
	void Shutdown();
	bool Init(); // musi se zavolat az kdyz je mozne nacitat textury
	void Render();
	void DrawUseText(const char* str);
private:
	IP3DFont* hudFnt;
	IP3DTexture *m_pTexUse;
	bool bLoaded;
public:
	bool bDrawTexUse;
};

extern CHUD g_HUD;