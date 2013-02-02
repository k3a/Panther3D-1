//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (4.7.2006)
// Purpose:	Transparent window - solid entity
//-----------------------------------------------------------------------------
#include "eInfo_window.h"

REGISTER_ENTITY(eInfo_window, "info_window", false, false, false);

eInfo_window::eInfo_window()
{
//	ENT_DATA_VEC("origin", m_vPos, 0, 0, 0, E_KEY_DONT_SAVE);
//	ENT_DATA_VEC("angles", m_vRot, 0, 0, 0, E_KEY_DONT_SAVE);
	//ENT_DATA("radius", m_nRadius, TYPE_INT, 0, E_KEY_DONT_SAVE);
}

eInfo_window::~eInfo_window()
{

}

bool eInfo_window::Spawn(geometryData_s& gdata)
{
	g_pAlphaMan = (IP3DAlphaManager*)I_GetClass(IP3DRENDERER_ALPHAMANAGER);

	for(UINT i=0;i<gdata.numFaces;i++)
	{
		if (gdata.faces[i].surfFlags!=19456) // FIXME: presne na flagy!!!
		{
			// vytvor okno
			g_pAlphaMan->CreateGlassWindow(gdata.faces[i].verts, gdata.faces[i].texture, NULL);
		}
	}
	return true;
}