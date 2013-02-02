//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Target_print (print text to client screen) entity class
//-----------------------------------------------------------------------------
//TODO: PREDELAT PRO NOVY ENTITOVY SYSTEM, UKLADAT NECO JAKO AKTUALNI CAS
//      A TIM ZARUCIT? ABY SE MESSAGE NEZOBRAZIL ZASE OD ZACATKU I PRI LOADU HRY
#include "eTarget_print.h"

REGISTER_ENTITY(eTarget_print, "target_print", false, false, true);

eTarget_print::eTarget_print()
{
	//registrace
	ENT_DATA("text", m_pSText, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edText");
	ENT_DATA("wait", m_pWait, TYPE_FLOAT, 3, E_KEY_DONT_SAVE, "#entities_edWait");

	m_pSText = NULL;
	m_pFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	m_pFont->CreateFont("Arial", 20, false, false);
	m_Tim.CreateTimer(2000); // default, je jedno jaka hodnota, se nastavi ve Spawn teprv

	CVr_width = g_pConsole->FindConVar("r_width");
	CVr_height = g_pConsole->FindConVar("r_height");
}

eTarget_print::~eTarget_print()
{
	SAFE_DELETE(m_pFont);
	SAFE_DELETE_ARRAY(m_pSText);
}
bool eTarget_print::Spawn(geometryData_s& gdata)
{
	m_Tim.SetTime(0, (unsigned long)m_pWait*1000/* timer to chce v ms*/);
	m_Tim.EnableTimer(0, false); // defaultne vypnout
	return true;
}

void eTarget_print::Render(float deltaTime) // aktualizace mozna do think?
{
	if (m_Tim.GetTimeRemaining(0) == 0 || strcmp(m_pSText, "") == 0) 
		m_Tim.EnableTimer(0, false);
	else
	{
		unsigned int col = (unsigned int)(255.0f / (m_Tim.GetTime(0)/2) * m_Tim.GetTimeRemaining(0));
		if (col > 200) col = 200;
		int lHei = m_pFont->GetLineH();
		m_pFont->DrawText(FA_LEFT, m_pSText, 71, (int)(CVr_height->GetInt()*0.9f-lHei/2.0f+1), CVr_width->GetInt(), CVr_height->GetInt(), P3DCOLOR_ARGB(col, 0, 0, 0)); // cerne pozadi
		m_pFont->DrawText(FA_LEFT, m_pSText, 70, (int)(CVr_height->GetInt()*0.9f-lHei/2.0f), CVr_width->GetInt(), CVr_height->GetInt(), P3DCOLOR_ARGB(col, 255, 255, 200));
	}
}

void eTarget_print::Activate(eBase* pCaller, int nValue)
{
	// TODO: tohle je od zacatku, je to tak?
	m_Tim.EnableTimer(0, true); // povolit timer
}
