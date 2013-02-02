//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Timer - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eMisc_timer : public eBase
{
public:
	eMisc_timer();
	bool Spawn(geometryData_s& gdata);
	void Think(float deltaTime);
	void Activate(eBase* pCaller, int nValue){m_bActive=nValue; m_fAktTimeSek=(-0.5f+rand())*2.0f*m_fRandom;};
private:
	int m_bActive;
	float m_fAktTimeSek;
	float m_fInterval;
	float m_fRandom;
	//
	EntEvent OnTimer;
};