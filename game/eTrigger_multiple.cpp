//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Trigger_multiple (trigger for entities) entity class
//-----------------------------------------------------------------------------
/*#include "eTrigger_multiple.h"
#include <math.h> // pro vypocet vzdalenosti

eTrigger_multiple::eTrigger_multiple()
{
}

eTrigger_multiple::~eTrigger_multiple()
{
}

bool eTrigger_multiple::Get(const epair_s *sEnt, int *numEntP)
{
	return true;
}

bool eTrigger_multiple::Set(const epair_s *sEnt, const int numEntP)
{	
	strcpy(m_strTarget, ValueForKey(sEnt, numEntP, "Target", "-"));
	VectorForKey(sEnt, numEntP, "origin", &m_vOrigin);

	VectorForKey(sEnt, numEntP, "minBB", &m_vMinBB);
	VectorForKey(sEnt, numEntP, "maxBB", &m_vMaxBB);
	return true;
}

void eTrigger_multiple::Loop(float deltaTime)
{
	// zjisti pozici hrace
	P3DPoint3D pPos;
	g_pCharCtrl->GetPos(pPos);

	//ECON(MSG_CON_DEBUG, "POZICE TRIGGERu: |%f|%f|%f|, POZICE HRACe: |%f|%f|%f|!", m_vOrigin.x, m_vOrigin.y, m_vOrigin.z, pPos.x, pPos.y, pPos.z);

	// zkontroluj zda je hrac v entite
	//if (sqrt(pow(pPos.x-m_vOrigin.x, 2)+pow(pPos.y-m_vOrigin.y, 2)+pow(pPos.z-m_vOrigin.z, 2)) < 100) // pokud je blize nez 1m
	if (pPos.x > m_vMinBB.x && pPos.y > m_vMinBB.y && pPos.z > m_vMinBB.z &&
		pPos.x < m_vMaxBB.x && pPos.y < m_vMaxBB.y && pPos.z < m_vMaxBB.z)
	{
		//ECON(MSG_CON_DEBUG, "trigger_multiple sestreluje cil!");
		// TODO: Pouzit wait - prodleva pred jednotlivymi "sestrelenimi" - prodleva pred opakovanim
		// player is in it
		g_pEntMgr->Fire(m_strTarget, ""); // TODO: parametry (prikaz), zatim je "" - nenastaven
	}
}

void eTrigger_multiple::Render(float deltaTime)
{
}

bool eTrigger_multiple::Fire(const char* strCmdName)
{
	// pokud bude sam "sestrelen", sestreli i cil
	g_pEntMgr->Fire(m_strTarget, ""); // TODO: parametry (prikaz), zatim je "" - nenastaven
	return true;
}
*/