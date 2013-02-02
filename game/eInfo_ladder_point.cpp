//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Ladder point - entity class
//-----------------------------------------------------------------------------
/*
Zebrik funguje tak, ze jsou dve tyto tridy, entity nad sebou kazda z nich urcuje jakysi bod zebriku. Kdyz hrac prijde k
nejakemu bodu, obevi se mu ikonka zebriku a pokud stiskne pouzit, dojde k prepnuti pohybu hrace, hrac se bude pohybovat
jen nahoru a dolu, nejdrive musi opustit stary bod. Jakmile se dostane mimo stary bod, tak se vlastne pohybuje po zebriku.
Jakmile dosahne nejakeho bodu pri tomto pohybu (klidne toho, kde nastupoval, nebo druheho, dojde k odpojeni od zebriku.
*/
#include "eInfo_ladder_point.h"

REGISTER_ENTITY(eInfo_ladder_point, "info_ladder_point", false, false, true);

static int m_nMoveState=0; // MOVE_NONE, MOVE_START nebo MOVE_MOVE
static UINT s_nLastID=0;
static UINT s_nActivatorID=0;

eInfo_ladder_point::eInfo_ladder_point()
{
	ENT_DATA("angle", m_nAngle, TYPE_INT, 0, E_KEY_DONT_SAVE, "#entities_edAngle");
	ENT_DATA("up", bUp, TYPE_INT, 0, E_KEY_DONT_SAVE, "#entities_edUp");

	ENT_EVENT("OnStartMove", OnStartMove, "#entities_eeOnStartMove");
	ENT_EVENT("OnEndMove", OnEndMove, "#entities_eeOnEndMove");
}

eInfo_ladder_point::~eInfo_ladder_point()
{

}

bool eInfo_ladder_point::Spawn(geometryData_s& gdata)
{
	g_pCharCtrl = (IP3DCharacterCtrl*)I_GetClass(IP3DPHYS_CHARACTERCONTROLLER);
	g_pEntMgr = (IP3DEntityMgr*)I_GetClass(IP3DGAME_ENTITYMGR);
	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);
	return true;
}

void eInfo_ladder_point::Finalize()
{
	// ziskej ukazatel na pozici hrace
	eBase* pEnt = g_pEntMgr->FindEntityByClassname("info_player_start");
	if(!pEnt) {Kill(this); return;}
	entVal_t* pVal = pEnt->m_Data.GetValue("origin");
	if(!pVal) {Kill(this); return;}
	m_vPlayerPos = pVal->vValue;

	// kvuli pozice char. ctrl. pricti 95
	//m_vPos.y += 164;
	m_nMoveState = MOVE_NONE;

	s_nLastID++;
	m_nID = s_nLastID;
}

void eInfo_ladder_point::Render(float deltaTime)
{
	GetWorldPos(m_vPos);

	// je hrac pobliz nejakeho bodu?
	// UPD: FIXME: nezobrazovat
	// ikonu ak je kamera otocena smerom od ladderu
	// pridat premennu m_vPlayerRot, rovnako ako je m_vPlayerPos
	if ((fabs (m_vPlayerPos->x - m_vPos.x) < (bUp ? ACTIVE_LADDER_UP_DISTANCE_HOR : ACTIVE_LADDER_DISTANCE_HOR)) && \
		(fabs (m_vPlayerPos->y - m_vPos.y) < ACTIVE_LADDER_DISTANCE_VER) && \
		(fabs (m_vPlayerPos->z - m_vPos.z) < (bUp ? ACTIVE_LADDER_UP_DISTANCE_HOR : ACTIVE_LADDER_DISTANCE_HOR)))
	{
		if (m_nMoveState == MOVE_NONE) // jeste nepouzil zebrik
		{
			g_HUD.bDrawTexUse = true;

			// zjisti, zda byla stisknuta klavesa pouzit
			if(g_pInput->IsKeyDownFirstTime((int)CVk_use.GetFloat()))
			{
				//ECON(MSG_ERR, "ZACATEK POHYBU PO ZEBRIKU!!!!");

				g_pCharCtrl->LadderBegin(m_vPos, 0.0f); // TODO: pouzit m_nAngle, jenze ono to nebude presne sedet. Nemuzu testovat, nemam zatim novy charctrl :)

				OnStartMove.RaiseEvent(this);
				m_nMoveState = MOVE_START;
				s_nActivatorID = m_nID;
			}
		}
		else if(m_nMoveState==MOVE_MOVE) // hrac se pohyboval po zebriku a dosahl poprve nejaky bod -> konec pohybu
		{
			//ECON(MSG_ERR, "KONEC POHYBU PO ZEBRIKU!!!!");

			g_pCharCtrl->LadderEnd(!!bUp);

			OnEndMove.RaiseEvent(this);
			m_nMoveState = MOVE_NONE; // uz je vlastne zase mimo zebrik
			s_nActivatorID = 0;
		}

		// test, aby hrac nesel dolu mimo rozsah zebriku
		if (!bUp && m_vPlayerPos->y < m_vPos.y-2.0f)
		{
			//ECON(MSG_ERR, "KONEC POHYBU PO ZEBRIKU (TEST DN)!!!!");
			g_pCharCtrl->LadderEnd(false);

			OnEndMove.RaiseEvent(this);
			m_nMoveState = MOVE_NONE; // uz je vlastne zase mimo zebrik
			s_nActivatorID = 0;
		}
		else if (bUp && m_vPlayerPos->y > m_vPos.y) // test, aby hrac nesel nahoru mimo rozsah zebriku
		{
			//ECON(MSG_ERR, "KONEC POHYBU PO ZEBRIKU (TEST UP)!!!!");
			g_pCharCtrl->LadderEnd(true);

			OnEndMove.RaiseEvent(this);
			m_nMoveState = MOVE_NONE; // uz je vlastne zase mimo zebrik
			s_nActivatorID = 0;
		}
	}
	else // hrac je mimo bod zebriku
	{
		if(m_nMoveState==MOVE_START && s_nActivatorID == m_nID) m_nMoveState=MOVE_MOVE; // poprve opustil jeden z bodu, ted se bude pohybovat, dokud nedorazi na nejaky bod
	}
}