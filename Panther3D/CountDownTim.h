//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Countdown Timer class definition
//-----------------------------------------------------------------------------

/************************************************************************

Odpoèítavací mìøic. 
Je možné nastavit nìkolik odpoèítavadel a získat jejich identifikátor.
Kdykoli je možné zjistit, zda již zadaný èas pro dané odpoèítavadlo
vypršel a popø. je možné ho ihned resetovat.

-pokud je m_startTime 0 je timer zakázán
-pokud je m_time 0 je timer volný, je možné ho použít
-SetTimer ho zároveò i resetuje == povolí
-Resetování == povolení (pak je možné zakázat hned po daném volání)

************************************************************************/

#pragma once

#include <windows.h>
#include "common.h"
#include "ip3dcountdowntimer.h"

#define MAX_TIMERS 100

//---------------------------------
class CP3DCountdownTimer : public IP3DCountdownTimer
{
public:
	CP3DCountdownTimer();
	~CP3DCountdownTimer();
	unsigned int CreateTimer(unsigned long numMs);
	void SetTime(unsigned int timerId, unsigned long numMs);
	bool IsTimeElapsed(unsigned int timerId, bool ifTrueResetTimer);
	void DeleteTimer(unsigned int timerId);
	void ResetTimer(unsigned int timerId);
	void EnableTimer(unsigned int timerId, bool bEnable);
	unsigned long GetTimeElapsed(unsigned int timerId);
	unsigned long GetTime(unsigned int timerId);
	unsigned long GetTimeRemaining(unsigned int timerId);
private:
	unsigned long m_startTime[MAX_TIMERS]; // zaèátek èasu, od kterého se mìøí daný timer (index)
	unsigned long m_time[MAX_TIMERS]; // doby timerù (index)
};
