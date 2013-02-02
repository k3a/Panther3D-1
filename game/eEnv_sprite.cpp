//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Sprite drawing - entity class
//-----------------------------------------------------------------------------
#include "eEnv_sprite.h"

REGISTER_ENTITY(eEnv_sprite, "env_sprite", false, false, true); // TODO: SORTING dle TEXTURY

eEnv_sprite::eEnv_sprite()
{
	ENT_DATA("texture", m_pszTexture, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edTexture");
	ENT_DATA_VEC("color", m_vColor, 255, 252, 255, E_KEY_SAVE, "#entities_edColor");
	ENT_DATA_VEC("scale", m_vScale, 1, 1, 1, E_KEY_DONT_SAVE, "#entities_edScale");
	ENT_DATA("alpha", m_nAlpha, TYPE_INT, 255, E_KEY_SAVE, "#entities_edAlpha");
}

eEnv_sprite::~eEnv_sprite()
{
	SAFE_DELETE(m_pTexture);
}

bool eEnv_sprite::Spawn(geometryData_s& gdata)
{
	if (!m_pszTexture) return false; // neni co nacitat

	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);

	m_pTexture = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE);
	char tmpPath[MAX_PATH];
	strcpy(tmpPath, m_pszTexture);
	if(!m_pTexture->Load(tmpPath, TT_2D)) return false; // nepodarilo se vytvorit texturu

	// uprav barvy
	if(m_vColor.x>255) m_vColor.x=255; if(m_vColor.y>255) m_vColor.y=255; if(m_vColor.z>255) m_vColor.z=255;
	if(m_nAlpha>255)m_nAlpha=255;

	// zjisti kde je stred textury
	m_pTexture->GetSize(&m_vCenter);
	m_vCenter.x /= 2.0f;
	m_vCenter.y /= 2.0f;

	// nastav docasnou matici
	m_mTmp.SetIdentityMatrix();
	m_mTmp.Scale(m_vScale.x, m_vScale.y, m_vScale.z);

	return true;
}

void eEnv_sprite::PreRender()
{
	g_p2D->Sprite_Begin(D3DXSPRITE_ALPHABLEND | SPRITE_OBJECTSPACE | SPRITE_BILLBOARD /*| SPRITE_SORT_DEPTH_BACKTOFRONT*/);
}

void eEnv_sprite::Render(float deltaTime)
{
	P3DPoint3D pnt;
	GetWorldPos(pnt);
	m_mTmp.m_posit.x = pnt.x; m_mTmp.m_posit.y = pnt.y; m_mTmp.m_posit.z = pnt.z;
	g_p2D->Sprite_SetTransform(m_mTmp);

	g_p2D->Sprite_Draw(m_pTexture, NULL, &m_vCenter, NULL, P3DCOLOR_ARGB(m_nAlpha, (int)m_vColor.x, (int)m_vColor.z, (int)m_vColor.y));
}

void eEnv_sprite::PostRender()
{
	g_p2D->Sprite_End();
}