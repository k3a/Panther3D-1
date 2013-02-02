//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (5.9.2006)
// Purpose:	Debug point entity - draws text in the screen space
//-----------------------------------------------------------------------------
#include "eDebug_Text.h"
#include "IP3DFont.h"

REGISTER_ENTITY(eDebug_text, "debug_text", false);

static IP3DFont *s_pFont;

eDebug_text::eDebug_text()
{
	ENT_DATA("text", m_pszText, TYPE_STRING, 0, E_KEY_DONT_SAVE, "#entities_edText");
	ENT_DATA("maxdist", m_nMaxDist, TYPE_INT, 7, E_KEY_DONT_SAVE, "#entities_edMaxdist");

	CVr_width = g_pConsole->FindConVar("r_width");
	CVr_height = g_pConsole->FindConVar("r_height");
}

eDebug_text::~eDebug_text()
{
	SAFE_DELETE(s_pFont);
}

bool eDebug_text::Spawn(geometryData_s& gdata)
{
	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);

	if (!m_pszText) 
	{
		ECON(MSG_CON_ERR, "Entity without text key!");
		return false;
	}
	
	if (!s_pFont) s_pFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	if (!s_pFont)
	{
		ECON(MSG_CON_ERR, "Can't get font class!");
		return false;
	}
	
	if (FAILED(s_pFont->CreateFont("Courier New", 17, false, false)))
	{
		ECON(MSG_CON_ERR, "Can't create font!");
		return false;
	}
	
	if (m_nMaxDist<=0)
	{
		ECON(MSG_CON_ERR, "max_dist <= 0! Turning back to 7!");
		m_nMaxDist = 7;
	}
	
	return true;
}

void eDebug_text::Render(float deltaTime)
{
	P3DXMatrix matView;
	P3DXMatrix matProj;
	g_pRenderer->GetTransform(RTT_VIEW, &matView);
	g_pRenderer->GetTransform(RTT_PROJECTION, &matProj);
	
	GetWorldPos(m_vPos);

	P3DXMatrix tmp;
	tmp.SetIdentityMatrix();
	tmp._41 = m_vPos.x; tmp._42 = m_vPos.y; tmp._43 = m_vPos.z;
	tmp *= matView;
	tmp *= matProj;
	
	float x=tmp._41/tmp._44;
	float y=tmp._42/tmp._44;
	float z=tmp._43/*/tmp._44*/;
	
	x = CVr_width->GetFloat()*(1+x)/2.0f;
	y = CVr_height->GetFloat()*(1-y)/2.0f;
	
	if (z > 0)
	{
		P3DScreenPoint2D drawPos;
		drawPos.x = (int)x;
		drawPos.y = (int)y;
		float a = 1.0f/(m_nMaxDist*100)*z;
		if (a < 1.0f) // mensi nez max. dist
			a = 255.0f;
		else if (a > 1.2f) // vetsi nez max dist + tlumeni
			a = 0.0f;
		else
		{
			a -= 1; // od 0 do 0.2 kde 0.2 ma byt 0lova alpha
			a = 255.0f - 255.0f / 0.2f * a;
		}
		
		if (a > 255) a = 255;
		if (a > 0) s_pFont->DrawText(m_pszText, drawPos, P3DCOLOR_ARGB((int)a, 255, 255, 255));
	}
}

bool eDebug_text::Command(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams)
{
	ECMDLIST_BEGIN
		ECMD_BEGIN(_EC_ENABLE)
			strcpy (m_pszText, "Enable called ;)");
		ECMD_END
		ECMD_BEGIN(_EC_DISABLE)
			strcpy (m_pszText, "Disable called ;)");
		ECMD_END
		ECMD_BEGIN(_EC_SETTEXT)
			strcpy (m_pszText, params[0].c);
		ECMD_END
	ECMDLIST_END
}