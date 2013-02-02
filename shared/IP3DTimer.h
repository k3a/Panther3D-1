//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Timer interface (engine)
//-----------------------------------------------------------------------------

#pragma once

#include "interface.h"

//---------------------------------
class IP3DTimer : public IP3DBaseInterface
{
public:
	virtual void Loop() = 0;
	virtual float GetTimeDelta_s() = 0;
	virtual DWORD GetTimeDelta_ms() = 0;
	virtual DWORD GetTimeDelta_micros() = 0;
	virtual DWORD GetTickCount_ms() = 0;
	virtual float GetTickCount_s() = 0;
};

#define IP3DENGINE_TIMER "P3DTimer_1" // nazev ifacu