//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Sprite drawing - entity class
//-----------------------------------------------------------------------------
#include "eLight_dynamic.h"

REGISTER_ENTITY(eLight_dynamic, "light_dynamic", false);

#define SF_SPAWN_LIGHT_OFF   (1 << 0)
#define SF_FLICKER_SMOOTH  (1 << 1)
#define SF_SPAWN_FLICKERING (1 << 2)

eLight_dynamic::eLight_dynamic()
{
	ENT_DATA("texture", m_pszTexture, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edTexture");
	ENT_DATA_VEC("color", m_vColor, 255, 252, 255, E_KEY_SAVE, "#entities_edColor");
	ENT_DATA("light", m_fRange, TYPE_FLOAT, 300, E_KEY_SAVE, "#entities_edLight");
	ENT_DATA("flicker_str", m_pszFlickerStr, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edFlickerStr"); //hccdeeeeeeeeeffffgilnpqqqommmmkjiihggggggghhijkllllrtvwwwvrrqomkjihhhiiiihhhffgikllmmmmmllpuutspmis
	ENT_DATA("flicker_cycle_time", m_fCycleTime, TYPE_FLOAT, 0.5f, E_KEY_DONT_SAVE, "#entities_edFlickerCycle");
	
	// sf
	ENT_DATA("SPAWN_LIGHT_OFF", m_bLightOff, TYPE_SF1, false, 0, "#entities_sfOff");
	ENT_DATA("FLICKER_SMOOTH", m_bFlickerSmooth, TYPE_SF2, false, 0, "#entities_sfFlickerSmooth");
	ENT_DATA("SPAWN_FLICKERING", m_bFlickering, TYPE_SF3, false, 0, "#entities_sfFlickering");
	
	// HACK
	ENT_DATA("rotate", rot, TYPE_INT, 0, E_KEY_DONT_SAVE, "HACK! FIXME! :D");
	angl = 0.0f;
	m_dwLight = 0xFFFFFFFF;
}

eLight_dynamic::~eLight_dynamic()
{
	if (g_pLightManager && (m_dwLight!=0xFFFFFFFF))
		g_pLightManager->RemoveLight (m_dwLight);
	SAFE_DELETE(m_pszTexture);
	SAFE_DELETE(m_pszFlickerStr);
}

bool eLight_dynamic::Spawn(geometryData_s& gdata)
{
	if (g_pLightManager == NULL) return false;

	DYN_LIGHT_INFO_USER	dynLight;
	P3DXMatrix	lightMat;
	P3DXVector3D pnt;
	GetWorldPos(pnt);

	lightMat.SetIdentityMatrix ();
	lightMat.Translate (pnt.x, pnt.y, pnt.z);
	dynLight.pos_rot = lightMat;
	dynLight.fRadius = m_fRange;
	dynLight.color = P3DColorValueRGB(m_vColor.x/255.0f, m_vColor.z/255.0f, m_vColor.y/255.0f); // x z y kvuli prehozeni vektoru
	dynLight.sz_light_texture = m_pszTexture;
	dynLight.bFlicker_smooth = m_bFlickerSmooth;
	dynLight.szFlicker_str = m_pszFlickerStr;
	dynLight.fFlicker_cycle_time = m_fCycleTime;
	m_dwLight = g_pLightManager->AddDynamicLight (dynLight, false);
	if (m_dwLight == 0xFFFFFFFF)
		return false;
	g_pLightManager->SwitchLight(m_dwLight, !m_bLightOff);
	g_pLightManager->SwitchFlickering(m_dwLight, m_bFlickering);


	return true;
}

void eLight_dynamic::Think(float deltaTime)
{
	GetWorldPos(m_vPos);

	// FIXME: GDS: KEX: HACK PRO ROTACI. NEJEDOU KLOUBY A NENI CAS TO RESIT
	if (rot)
	{
		P3DXMatrix m;
		m.SetIdentityMatrix();
		angl -= P3D_PI/7.5f * deltaTime;
		if (angl>2.0f*P3D_PI) angl -= 2.0f*P3D_PI;
		m.RotateY(angl);
		m.Translate(m_vPos.x, m_vPos.y, m_vPos.z);
		g_pLightManager->SetLightPosition(m_dwLight, m);
		return;
	}


	P3DXVector3D pnt;
	if (GetWorldPos(pnt))
	{
		P3DXMatrix lightMat;
		lightMat.SetIdentityMatrix ();
		lightMat.Translate (pnt.x, pnt.y, pnt.z);
		g_pLightManager->SetLightPosition(m_dwLight, lightMat);
	}
}