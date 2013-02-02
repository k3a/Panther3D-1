//------------ Copyright � 2005-2006 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Countdown Timer interface (game)
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
#include "common.h"
#include "interface.h"

//---------------------------------
class IP3DCountdownTimer : public IP3DBaseInterface
{
public:
	virtual unsigned int CreateTimer(unsigned long numMs)=0;
	virtual void SetTime(unsigned int timerId, unsigned long numMs)=0;
	virtual bool IsTimeElapsed(unsigned int timerId, bool ifTrueResetTimer)=0;
	virtual void DeleteTimer(unsigned int timerId)=0;
	virtual void ResetTimer(unsigned int timerId)=0;
	virtual void EnableTimer(unsigned int timerId, bool bEnable)=0;
	virtual unsigned long GetTimeElapsed(unsigned int timerId)=0;
	virtual unsigned long GetTime(unsigned int timerId)=0;
	virtual unsigned long GetTimeRemaining(unsigned int timerId)=0;
};

#define IP3DENGINE_COUNTDOWNTIMER "P3DCountdownTimer_1" // nazev ifacu