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
#include "CountDownTim.h"

REGISTER_CLASS(CP3DCountdownTimer, IP3DENGINE_COUNTDOWNTIMER); // zaregistruj tuto tøídu

CP3DCountdownTimer::CP3DCountdownTimer()
{
	int i;
	for (i=0;i<MAX_TIMERS;i++) // vynulovat
	{
		m_startTime[i]=0;
		m_time[i]=0;
	}
}

CP3DCountdownTimer::~CP3DCountdownTimer()
{
}

unsigned int CP3DCountdownTimer::CreateTimer(unsigned long numMs)
{
	int i;
	for(i=0; i<MAX_TIMERS; i++)
	{
		if (m_time[i]==0) 
		{
			m_time[i] = numMs; // nastav dobu
			m_startTime[i] = GetTickCount(); // nastav aktualni pocet ticku
			return i;
		}
	}
	CON(MSG_CON_INFO, "Maximal number of countdown timers was exceeded!");
	return MAX_TIMERS;
}

void CP3DCountdownTimer::SetTime(unsigned int timerId, unsigned long numMs)
{
	if (timerId >= MAX_TIMERS) return;
	m_time[timerId] = numMs;
	m_startTime[timerId] = GetTickCount();
}

bool CP3DCountdownTimer::IsTimeElapsed(unsigned int timerId, bool ifTrueResetTimer)
{
	if (timerId >= MAX_TIMERS || m_startTime[timerId] == 0) return false; // ...nebo pokud je zakázán
	if ((GetTickCount() - m_startTime[timerId]) >= m_time[timerId]) // èas uplynul
	{
		if (ifTrueResetTimer) m_startTime[timerId] = GetTickCount(); // reset timer
		return true;
	}
	return false;
}

void CP3DCountdownTimer::DeleteTimer(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return;
	m_time[timerId] = 0;
	m_startTime[timerId] = 0;
}

void CP3DCountdownTimer::ResetTimer(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return;
	m_startTime[timerId] = GetTickCount();
}

void CP3DCountdownTimer::EnableTimer(unsigned int timerId, bool bEnable)
{
	if (bEnable)
		m_startTime[timerId] = GetTickCount(); // povolit;
	else
		m_startTime[timerId] = 0; // zakázat
}

unsigned long CP3DCountdownTimer::GetTimeElapsed(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return 0;
	return GetTickCount() - m_startTime[timerId]; // uplynuly cas
}

unsigned long CP3DCountdownTimer::GetTime(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return 0;
	return m_time[timerId]; // vratit cas odpocitavadla
}

unsigned long CP3DCountdownTimer::GetTimeRemaining(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return 0;
	return m_time[timerId] - (GetTickCount() - m_startTime[timerId]); // zbyvajici cas
}