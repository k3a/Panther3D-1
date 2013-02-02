//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Timer - entity class
//-----------------------------------------------------------------------------
#include "eMisc_timer.h"

REGISTER_ENTITY(eMisc_timer, "misc_timer", false, true, false);

#define SF_SPAWN_ENABLED (1 << 0)

eMisc_timer::eMisc_timer()
{
	ENT_DATA("active", m_bActive, TYPE_INT, 0, E_SAVE, "");
	ENT_DATA("akt_time", m_fAktTimeSek, TYPE_FLOAT, 0, E_SAVE, "");
	ENT_DATA("interval", m_fInterval, TYPE_FLOAT, 1, E_KEY_DONT_SAVE, "#entities_edInterval");
	ENT_DATA("random", m_fRandom, TYPE_FLOAT, 0, E_KEY_DONT_SAVE, "#entities_edRandom");
	//
	ENT_EVENT("OnTimer", OnTimer, "#entities_eeOnAction");
	//
	ENT_DATA("SPAWN_ENABLED", m_bActive, TYPE_SF1, true, 0, "#entities_sfEnabled");
	// TODO: inicializace generatoru nahodnych cisel
}

bool eMisc_timer::Spawn(geometryData_s& gdata)
{
	m_fAktTimeSek=(-0.5f+rand())*2.0f*m_fRandom; // vynuluj na nahodnou odchylku

	return true;
}

void eMisc_timer::Think(float deltaTime)
{
	if(!m_bActive) return; // je vypnut

	m_fAktTimeSek += deltaTime; // pricti
	if (m_fAktTimeSek > m_fInterval)
	{
		OnTimer.RaiseEvent(this);
		m_fAktTimeSek=(-0.5f+rand())*2.0f*m_fRandom; // vynuluj na nahodnou odchylku
	}
}