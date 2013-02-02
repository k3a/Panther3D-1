//----------- Copyright � 2005-2007, 7thSquad, all rights reserved ------------
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

Odpo��tavac� m��ic. 
Je mo�n� nastavit n�kolik odpo��tavadel a z�skat jejich identifik�tor.
Kdykoli je mo�n� zjistit, zda ji� zadan� �as pro dan� odpo��tavadlo
vypr�el a pop�. je mo�n� ho ihned resetovat.

-pokud je m_startTime 0 je timer zak�z�n
-pokud je m_time 0 je timer voln�, je mo�n� ho pou��t
-SetTimer ho z�rove� i resetuje == povol�
-Resetov�n� == povolen� (pak je mo�n� zak�zat hned po dan�m vol�n�)

************************************************************************/

#pragma once

#include <windows.h>

#define MAX_TIMERS 100

//---------------------------------
class CCountdownTimer
{
public:
	CCountdownTimer();
	~CCountdownTimer();
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
	unsigned long m_startTime[MAX_TIMERS]; // za��tek �asu, od kter�ho se m��� dan� timer (index)
	unsigned long m_time[MAX_TIMERS]; // doby timer� (index)
};
