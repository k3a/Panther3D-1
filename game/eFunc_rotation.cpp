//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Rotation. Entity class.
//-----------------------------------------------------------------------------
#include "eFunc_rotation.h"

REGISTER_ENTITY(eFunc_rotation, "func_rotation", false, true, false);

#define SF_SPAWN_ENABLED (1 << 0)

eFunc_rotation::eFunc_rotation()
{
	//registrace
	ENT_DATA_VEC("omega", m_vOmega, 0, 0, 0, E_KEY_DONT_SAVE, "#entities_edOmega"); // FIXME: pokud bude potreba promenlive rychlosti, ukladat
	ENT_DATA("target", m_szTarget, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edTarget");

	ENT_DATA("SPAWN_ENABLED", m_bEnabled, TYPE_SF1, true, 0, "#entities_sfEnabled");
}

bool eFunc_rotation::Spawn(geometryData_s& gdata)
{
	return true;
}

void eFunc_rotation::Finalize()
{
	// najdi cil
	eBase* pEnt = g_pEntMgr->FindEntityByTargetname(m_szTarget);
	if(!pEnt) {Kill(this); return;} // nema co otacet, zde musime zavolat kill

	// najdi klice
	entVal_t* pBody = pEnt->m_Data.GetValue("body");
	if(!pBody) {Kill(this); return;} // nema co nastavovat

	// ziskej teleso
	m_pBody = *(IP3DRigidBody**)pBody->pValue;
	if(!m_pBody) {Kill(this); return;}
}

void eFunc_rotation::Think(float deltaTime)
{
	if(!m_bEnabled) return;
	m_pBody->SetOmega(m_vOmega);
}