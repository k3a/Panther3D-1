//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	High resolution timer definition
//-----------------------------------------------------------------------------



/************************************************************************

 meria presny cas medzi poslednymi dvoma volaniami fcie Loop()
 nehodi sa pre meranie vacsich casovych usekov - 10 min a viac

************************************************************************/


#pragma once

#include <windows.h>
#include "common.h"
#include "..\shared\IP3DTimer.h"


//---------------------------------
class CP3DTimer : public IP3DTimer
{
private:
	LARGE_INTEGER	m_liLastValue;
	float			m_fFreq;				// performance counter frequency
	float			m_fElapsedTimeSec;		// cas v sekundach od posledneho volania funkcie Loop
	DWORD			m_dwElapsedTimeMiliSec;	// cas v milisekundach
	DWORD			m_dwElapsedTimeMicroSec;// cas v mikrosekundach
	DWORD			m_dwGetTickCount;		// cas v milisekundach od spustenie systemu, aktualizuje sa v Loop()
public:
	CP3DTimer();
	~CP3DTimer();
	void Loop();
	float GetTimeDelta_s();
	DWORD GetTimeDelta_ms();
	DWORD GetTimeDelta_micros();
	DWORD GetTickCount_ms();
	float GetTickCount_s();
};
