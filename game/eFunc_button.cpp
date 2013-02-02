//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Button - entity class
//-----------------------------------------------------------------------------
// TODO:
// - zprehlednit, 
// - udelat spawnflag SPAWN_ON a klic switch a udalosti OnTrue a OnFalse a podle
//   toho pouste zvuky od a off, jinak jako tlacitko
#include "eFunc_button.h"
#include "common.h"
#include "math.h"

REGISTER_ENTITY(eFunc_button, "func_button", false);

eFunc_button::eFunc_button()
{
	ENT_DATA("wait", m_fWait, TYPE_FLOAT, 0, E_KEY_DONT_SAVE, "#entities_edWait");
	ENT_DATA("radius", m_fRadius, TYPE_FLOAT, 15.0f, E_KEY_DONT_SAVE, "#entities_edRadius");
	ENT_DATA("text", m_szMessage, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edText");

	ENT_EVENT("OnPress", OnPress, "#entities_eeOnPress");

	m_fTime = 0.0f;
	bPressed = false;
	bDrawUseButton = false;

	g_pEntMgr = (IP3DEntityMgr*)I_GetClass(IP3DGAME_ENTITYMGR);
	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);
	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
}

eFunc_button::~eFunc_button()
{

}

bool eFunc_button::Spawn(geometryData_s& gdata)
{
	P3DAABB aabb(-m_fRadius, -m_fRadius, -m_fRadius, m_fRadius, m_fRadius, m_fRadius);
	SetRelAABB(aabb);

	m_sndOn = g_pSoundMan->LoadSound ("button_on");
	//m_sndOff = g_pSoundMan->LoadSound ("button_off");

	if (!m_sndOn /*|| !m_sndOff*/) 
	{
		ECON(MSG_CON_ERR, "Can't load button sounds!");
		return false;
	}

	return true;
}

void eFunc_button::Finalize()
{
	// ziskej ukazatel na pozici hrace
	pPlayer = g_pEntMgr->FindEntityByClassname("info_player_start");
	if(!pPlayer) {Kill(this); return;}

	// lookat
	entVal_t* pVal = pPlayer->m_Data.GetValue("lookat");
	if(!pVal) {Kill(this); return;}
	m_vPlayerLookat = pVal->vValue;

	m_fTime = 0.0f;
}

void eFunc_button::Render(float deltaTime)
{
	if (bDrawUseButton) 
	{
		g_HUD.bDrawTexUse = true;
		if (m_szMessage) g_HUD.DrawUseText(m_szMessage);
	}
}

void eFunc_button::Think(float deltaTime)
{
	pPlayer->GetWorldPos(m_vPlayerPos);
	GetWorldPos(m_vPos);
	
	bDrawUseButton = false;

	if (m_fTime > 0) m_fTime -= deltaTime; // aktualizuj cas

	P3DXVector3D pLook = *m_vPlayerLookat;
	P3DXVector3D pPos = m_vPlayerPos;
	P3DXVector3D pVec = pLook-pPos;
	pVec.Normalize();

	P3DRay ray(m_vPlayerPos, pVec);
	P3DSphere sph(m_vPos, m_fRadius);

	float dist = Ray2SphereDist(ray, sph);
	if (dist > 0.0f && dist < 75.0f)
	{
		if(m_fTime <= 0.0f && !bPressed) // je mozno stisknout
		{
			// zjisti, zda byla stisknuta klavesa pouzit
			if(g_pInput->IsKeyDownFirstTime((int)CVk_use.GetFloat()))
			{
				if (m_fWait==-1.0f) bPressed = true; // tlacitko typu "lze stisknout jen jednou"?

				m_fTime = m_fWait;
				g_pSoundMan->PlaySound (m_sndOn);
				OnPress.RaiseEvent(this);
			}

			bDrawUseButton = true;
			g_HUD.bDrawTexUse = true;
		}
	}
}