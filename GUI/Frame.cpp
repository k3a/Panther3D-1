//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Frame (Group)
//-----------------------------------------------------------------------------
#include "Frame.h"

CFrame::CFrame()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_bDrawBorder = true;

	m_szCaption = new char[11];
	strcpy(m_szCaption, "Frame");

	SetElementFlags(EF_LINE | EF_SPRITE); // kreslime line + text
}

bool CFrame::CreateFrame(const char* szCaption, int x, int y, int w, int h, bool bDrawBorder, IGuiElement* pParent)
{
	// nastav parentu jako child tento prvek
	pParent->SetChild(this);
	m_pParent = pParent;

	// nastav
	if(szCaption)
	{
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	}
	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;
	m_bDrawBorder = bDrawBorder;

	return true;
}

void CFrame::RenderLine(float deltaTime)
{
	// vykresli ohraniceni
	if(m_bDrawBorder)
	{
		int w, h;
		g_Scheme.GetFontLabel()->GetTextSize(m_szCaption, NULL, &w, &h);
		g_p2D->Line_DrawRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+2+h/2, m_nWidth-1, m_nHeight-2, g_Scheme.GetColor(COL_LIGHT));
		g_p2D->Line_DrawRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY+1+h/2, m_nWidth-1, m_nHeight-2, g_Scheme.GetColor(COL_LIGHTSHADOW));
		g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+6, m_nPosY+m_nParentY+1, w+4, h, g_Scheme.GetColor(COL_BG));
	}
}

void CFrame::RenderSprite(float deltaTime)
{
	// VYKRESLI TEXT
	if(m_bDrawBorder) g_Scheme.GetFontLabel()->DrawText(FA_LEFT, m_szCaption, (int)(m_nPosX+m_nParentX+8), (int)(m_nPosY+m_nParentY+1), (int)(m_nPosX+m_nParentX+m_nWidth-3), (int)(m_nPosY+m_nParentY+12), g_Scheme.GetColor(COL_TEXT));
}