//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Timer class definition
//-----------------------------------------------------------------------------

#include "timer.h"

REGISTER_SINGLE_CLASS(CP3DTimer, IP3DENGINE_TIMER);

ConVar CVe_timer_speed("e_timer_speed", "1", CVAR_NOT_STRING, "Zmena rychlosti timeru hry");


float g_fChange = 1.0f;

//---------------------------------
void TimerSpeedChangeCallack (ConVar* pCVar)
{
	g_fChange = pCVar->GetFloat();
}

//---------------------------------
CP3DTimer::CP3DTimer()
{
	LARGE_INTEGER lint;
	m_fFreq = 1.0f;
	if (QueryPerformanceFrequency (&lint))
		m_fFreq = float(lint.LowPart);
	if (!QueryPerformanceCounter (&m_liLastValue))
		m_liLastValue.LowPart = GetTickCount ();
	m_fElapsedTimeSec = 0;
	m_dwElapsedTimeMiliSec = 0;
	m_dwGetTickCount = GetTickCount ();
	CVe_timer_speed.SetChangeCallback (TimerSpeedChangeCallack, false);
}

//---------------------------------
CP3DTimer::~CP3DTimer()
{}

//---------------------------------
void CP3DTimer::Loop()
{
	LARGE_INTEGER lint;

	if (g_pConsole->IsShown ()) return;

	m_dwGetTickCount = GetTickCount ();

	if (QueryPerformanceCounter (&lint))
	{
		DWORD dwTimeDif = lint.LowPart - m_liLastValue.LowPart;
		m_fElapsedTimeSec = float(dwTimeDif) / m_fFreq;
		m_dwElapsedTimeMiliSec = DWORD(m_fElapsedTimeSec * 1000.0f);
		m_dwElapsedTimeMicroSec = DWORD(m_fElapsedTimeSec * 1000000.0f);
		m_liLastValue.LowPart = lint.LowPart;
	}
	else
	{
		// ak hardware nepodporuje high-resolution performance counter pouzije sa GetTickCount()
		m_dwElapsedTimeMiliSec = m_dwGetTickCount - m_liLastValue.LowPart;
		m_dwElapsedTimeMicroSec = m_dwElapsedTimeMiliSec * 1000;
		m_fElapsedTimeSec =  m_dwElapsedTimeMiliSec * 0.001f;
		m_liLastValue.LowPart = m_dwGetTickCount;
	}

	if (g_fChange != 1.0f)
	{
		m_fElapsedTimeSec *= g_fChange;
		m_dwElapsedTimeMiliSec = DWORD (float(m_dwElapsedTimeMiliSec) * g_fChange);
		m_dwElapsedTimeMicroSec = DWORD (float(m_dwElapsedTimeMicroSec) * g_fChange);
		m_dwGetTickCount = DWORD (float(m_dwGetTickCount) * g_fChange);
	}
}

//---------------------------------
float CP3DTimer::GetTimeDelta_s() {
	return m_fElapsedTimeSec;
}
DWORD CP3DTimer::GetTimeDelta_ms() {
	return m_dwElapsedTimeMiliSec;
}
DWORD CP3DTimer::GetTimeDelta_micros() {
	return m_dwElapsedTimeMicroSec;
}

// cas v milisekundach od spustenia systemu, aktualizuje sa v Loop()
DWORD CP3DTimer::GetTickCount_ms() {
	return m_dwGetTickCount;
}

// presny okamzity cas
float CP3DTimer::GetTickCount_s()
{
	LARGE_INTEGER lint;
	float fRet;
	if (QueryPerformanceCounter (&lint))
		fRet = float(lint.LowPart) / m_fFreq;
	else
		fRet = (float)GetTickCount ();

	if (g_fChange != 1.0f)
		fRet *= g_fChange;

	return fRet;
}
