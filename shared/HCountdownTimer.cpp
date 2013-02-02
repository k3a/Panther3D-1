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
#include "HCountDownTimer.h"

CCountdownTimer::CCountdownTimer()
{
	int i;
	for (i=0;i<MAX_TIMERS;i++) // vynulovat
	{
		m_startTime[i]=0;
		m_time[i]=0;
	}
}

CCountdownTimer::~CCountdownTimer()
{
}

unsigned int CCountdownTimer::CreateTimer(unsigned long numMs)
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
	MessageBox(0, "Maximal number of countdown timers was exceeded!\r\nCan't create this timer!!!", "Error!", MB_SYSTEMMODAL);
	return MAX_TIMERS;
}

void CCountdownTimer::SetTime(unsigned int timerId, unsigned long numMs)
{
	if (timerId >= MAX_TIMERS) return;
	m_time[timerId] = numMs;
	m_startTime[timerId] = GetTickCount();
}

bool CCountdownTimer::IsTimeElapsed(unsigned int timerId, bool ifTrueResetTimer)
{
	if (timerId >= MAX_TIMERS || m_startTime[timerId] == 0) return false; // ...nebo pokud je zakázán
	if ((GetTickCount() - m_startTime[timerId]) >= m_time[timerId]) // èas uplynul
	{
		if (ifTrueResetTimer) m_startTime[timerId] = GetTickCount(); // reset timer
		return true;
	}
	return false;
}

void CCountdownTimer::DeleteTimer(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return;
	m_time[timerId] = 0;
	m_startTime[timerId] = 0;
}

void CCountdownTimer::ResetTimer(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return;
	m_startTime[timerId] = GetTickCount();
}

void CCountdownTimer::EnableTimer(unsigned int timerId, bool bEnable)
{
	if (bEnable)
		m_startTime[timerId] = GetTickCount(); // povolit;
	else
		m_startTime[timerId] = 0; // zakázat
}

unsigned long CCountdownTimer::GetTimeElapsed(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return 0;
	return GetTickCount() - m_startTime[timerId]; // uplynuly cas
}

unsigned long CCountdownTimer::GetTime(unsigned int timerId)
{
	if (timerId >= MAX_TIMERS) return 0;
	return m_time[timerId]; // vratit cas odpocitavadla
}

unsigned long CCountdownTimer::GetTimeRemaining(unsigned int timerId) // POZOR! Vrati 0 pokud je disabled, používat IsTimeElapsed místo (GetTimeRemaining==0)
{
	if (timerId >= MAX_TIMERS) return 0;
	if (m_startTime[timerId]==0) return 0;
	long lTmp = m_time[timerId] - (GetTickCount() - m_startTime[timerId]);
	if (lTmp<0) lTmp = 0;
	return lTmp; // zbyvajici cas
}