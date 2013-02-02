//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Hinge phys. constraint - entity class
//-----------------------------------------------------------------------------
#include "ePhys_hinge.h"

REGISTER_ENTITY(ePhys_hinge, "phys_hinge", false, true, false);

ePhys_hinge::ePhys_hinge()
{
	ENT_DATA("target1", m_pszTarget1, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edTarget");
	ENT_DATA("target2", m_pszTarget2, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edTarget");
	ENT_DATA("maxforce", m_fMaxForce, TYPE_FLOAT, 0.0f, E_KEY_DONT_SAVE, "#entities_edMaxforce");
	ENT_DATA_VEC("direction", m_vDir, 0, 0, 0, E_KEY_DONT_SAVE, "#entities_edDirection");

	ENT_EVENT("OnBreak", OnBreak, "#entities_eeOnBreak");
	
	m_pJoint = NULL;
}

ePhys_hinge::~ePhys_hinge()
{
	SAFE_DELETE_ARRAY(m_pszTarget1);
	SAFE_DELETE_ARRAY(m_pszTarget2);
	//
	if(m_pJoint) g_pNewtonWorld->ConstraintDestroy(m_pJoint);
}

bool ePhys_hinge::Spawn(geometryData_s& gdata)
{
	g_pNewtonWorld = (IP3DPhysWorld*)I_GetClass(IP3DPHYS_PHYSWORLD);
	return !!g_pNewtonWorld;
}

void ePhys_hinge::Finalize()
{
	GetWorldPos(m_vPos);

	if(!m_pszTarget1 && !m_pszTarget2) // pokud jsou cile
	{
		ECON(MSG_CON_DEBUG, "Entity without joint targets!");
		Kill(this);
	}

	// najdi ukazatele na entity - cily
	eBase* pEnt1 = g_pEntMgr->FindEntityByTargetname(m_pszTarget1); 
	eBase* pEnt2 = g_pEntMgr->FindEntityByTargetname(m_pszTarget2);
	if(!pEnt1 && !pEnt2) // pokud jsou cile
	{
		ECON(MSG_CON_DEBUG, "Can't find joint targets!");
		Kill(this);
	}

	// ziskej data
	entVal_t* pBody1=NULL; if(pEnt1) pBody1 = pEnt1->m_Data.GetValue("body");
	entVal_t* pBody2=NULL; if(pEnt2) pBody2 = pEnt2->m_Data.GetValue("body");
	if(!pBody1 && !pBody2) {Kill(this); return;} // nema co nastavovat

	// ziskej konkretni ukazatele
	IP3DRigidBody* m_pBody1 = NULL;
	IP3DRigidBody* m_pBody2 = NULL;
	if(pBody1) m_pBody1 = *(IP3DRigidBody**)pBody1->pValue;
	if(pBody2) m_pBody2 = *(IP3DRigidBody**)pBody2->pValue;
	if(!m_pBody1 && !m_pBody2) {Kill(this); return;} // oba nulove ukazatele

	// ok... vytvor omezeni
	if (!m_pBody1) // pokud je 1. NULL, musi byt nastaven jako child ten druhy
		g_pNewtonWorld->ConstraintCreateHinge(&m_pJoint, m_vPos, m_vDir, m_pBody2, m_pBody1, m_fMaxForce);
	else // je to naopak nebo jsou to 2 platne telesa
		g_pNewtonWorld->ConstraintCreateHinge(&m_pJoint, m_vPos, m_vDir, m_pBody1, m_pBody2, m_fMaxForce);

	if(!m_pJoint) Kill(this);
}

void ePhys_hinge::Think(float deltaTime)
{
	if (!m_pJoint) // joint jiz neexistuje, byl znicen, stalo se to poprve, protoze entita zije
	{ 
		OnBreak.RaiseEvent(this); 
		Kill(this); // uz nebude dochazet k Thinku
	}
}

void ePhys_hinge::Break(eBase* pCaller, int nValue)
{
	// znicit joint a ukoncit entitu
	g_pNewtonWorld->ConstraintDestroy(m_pJoint);
	m_pJoint = NULL;

	OnBreak.RaiseEvent(this); // doslo ke zlomeni

	Kill(this);
}