//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Label
//-----------------------------------------------------------------------------
#include "Label.h"

CLabel::CLabel()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_bBold = false;
	m_eAlign = LA_LEFT;

	m_szCaption = new char[11];
	strcpy(m_szCaption, "Label");
	
	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_SPRITE); // kreslime text
}

bool CLabel::CreateLabel(const char* szCaption, int x, int y, int w, int h, bool bBold, int align, IGuiElement* pParent)
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

	m_bBold = bBold;
	m_eAlign = align;

	return true;
}

void CLabel::RenderSprite(float deltaTime)
{
	// VYKRESLI TEXT
	if (m_bBold)
		g_Scheme.GetFontLabelBold()->DrawText(m_eAlign, m_szCaption, (int)(m_nPosX+m_nParentX), (int)(m_nPosY+m_nParentY), (int)(m_nPosX+m_nParentX+m_nWidth), (int)(m_nPosY+m_nParentY+m_nHeight), g_Scheme.GetColor(COL_TEXT));
	else
		g_Scheme.GetFontLabel()->DrawText(m_eAlign, m_szCaption, (int)(m_nPosX+m_nParentX), (int)(m_nPosY+m_nParentY), (int)(m_nPosX+m_nParentX+m_nWidth), (int)(m_nPosY+m_nParentY+m_nHeight), g_Scheme.GetColor(COL_TEXT));
}

void CLabel::Update(float deltaTime)
{
	// GUI EDITOR
	if(CVgui_editor.GetBool() && !(GetElementFlags() & EF_NOT_EDITABLE))
	{
		if (!m_szName) return; // neoznacene prvky nelze upravovat

		// popisek
		char str[64];
		bool popisek=false;

		if (g_bLMouseDownFirstTime)
		{
			if ((g_nMouseX>m_nPosX+m_nParentX+m_nWidth-3 && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth)
				&& (g_nMouseY>m_nPosY+m_nParentY+m_nHeight-3 && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight))
			{
				edit_mode = 2; // resize
			}
			else if (g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
				&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight)
			{
				edit_mode = 1; // move
				edit_nRelPosX = g_nMouseX - m_nPosX;
				edit_nRelPosY = g_nMouseY - m_nPosY;
			}
		}
		else if (g_bLMouseUpFirstTime)
		{
			edit_mode = 0; // konec
		}

		// pokud je co delat, tak to udelej
		if (edit_mode==1) // move
		{
			m_nPosX = g_nMouseX - edit_nRelPosX;
			m_nPosY = g_nMouseY - edit_nRelPosY;
			sprintf(str, "X%d Y%d", m_nPosX, m_nPosY);
			g_Scheme._pLabelFont->DrawText(str, g_nMouseX+10, g_nMouseY+10, P3DCOLOR_XRGB(255, 255, 255));
			popisek = true;
		}
		else if (edit_mode==2) // resize
		{
			m_nWidth = g_nMouseX - m_nPosX - m_nParentX;
			m_nHeight = g_nMouseY - m_nPosY - m_nParentY;
			sprintf(str, "W%d H%d", m_nWidth, m_nHeight);
			g_Scheme._pLabelFont->DrawText(str, g_nMouseX+10, g_nMouseY+10, P3DCOLOR_XRGB(255, 255, 255));
			popisek = true;
		}

		// pokud neni zaden popisek a mys je nad timto prvkem, udelej popisek
		if (g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
			&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight)
		{
			// popisek
			char str[64];
			if (!popisek)
			{
				// vykresli okraje
				g_p2D->Line_DrawRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, P3DCOLOR_ARGB(100, 0, 255, 0));

				sprintf(str, "%s", m_szName);
				g_Scheme._pLabelFont->DrawText(str, g_nMouseX+10, g_nMouseY+10, P3DCOLOR_ARGB(100, 255, 255, 255));
			}
		}
		return; // editor mode
	}
	//-GUI EDITOR
}