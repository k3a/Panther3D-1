//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (13.10.2006)
// Purpose:	Transparent grass - entity
//-----------------------------------------------------------------------------
#include "eEnv_grass.h"

REGISTER_ENTITY(eEnv_grass, "env_grass", false, false, false);

eEnv_grass::eEnv_grass()
{
		ENT_DATA("radius", m_fRadius, TYPE_FLOAT, 200.0f, E_KEY_DONT_SAVE, "#entities_edRadius");
		ENT_DATA("count", m_nQuadCnt, TYPE_INT, 120, E_KEY_DONT_SAVE, "#entities_edCount");
		ENT_DATA("maxdist", m_fDist, TYPE_FLOAT, 1000.0f, E_KEY_DONT_SAVE, "#entities_edMaxdist");
		ENT_DATA("minsize", m_fMinSize, TYPE_FLOAT, 40.0f, E_KEY_DONT_SAVE, "#entities_edMinsize");
		ENT_DATA("maxsize", m_fMaxSize, TYPE_FLOAT, 80.0f, E_KEY_DONT_SAVE, "#entities_edMaxsize");
		ENT_DATA("texture", m_strTexture, TYPE_STRING, "textures\\grass\\grass.dds", E_KEY_DONT_SAVE, "#entities_edTexture");

		ENT_DATA("ANIMATE", bAnimate, TYPE_SF1, false, E_SPAWNFLAG, "#entities_sfAnimate");
		ENT_DATA("BILLBOARDED", bBillboarded, TYPE_SF2, true, E_SPAWNFLAG, "#entities_sfBillboarded");
}

eEnv_grass::~eEnv_grass()
{

}

bool eEnv_grass::Spawn(geometryData_s& gdata)
{
	GetWorldPos(m_vOrigin);
	g_pAlphaMan = (IP3DAlphaManager*)I_GetClass(IP3DRENDERER_ALPHAMANAGER);
	g_pAlphaMan->CreateGrassBunch ((P3DPoint3D)m_vOrigin, (float)m_fRadius, m_nQuadCnt, m_fDist, m_fMinSize, m_fMaxSize, \
				                    m_strTexture, bBillboarded, bAnimate);
	return true;
}